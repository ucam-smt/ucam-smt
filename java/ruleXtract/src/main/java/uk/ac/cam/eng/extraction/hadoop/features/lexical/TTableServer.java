/*******************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use these files except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright 2014 - Juan Pino, Aurelien Waite, William Byrne
 *******************************************************************************/
package uk.ac.cam.eng.extraction.hadoop.features.lexical;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.zip.GZIPInputStream;

import org.apache.commons.lang.time.StopWatch;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.util.StringUtils;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.util.Util;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.Parameters;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class TTableServer extends Configured implements Closeable, Tool {

	final static int BUFFER_SIZE = 65536;

	private ExecutorService threadPool = Executors.newFixedThreadPool(6);

	private class LoadTask implements Runnable {

		private final String fileName;
		private final byte prov;

		private LoadTask(String fileName, byte prov) {
			this.fileName = fileName;
			this.prov = prov;
		}

		@Override
		public void run() {
			try {
				loadModel(fileName, prov);
			} catch (IOException e) {
				e.printStackTrace();
				System.exit(1);
			}

		}

	}

	private class QueryRunnable implements Runnable {

		private Socket querySocket;

		private ByteArrayOutputStream byteBuffer = new ByteArrayOutputStream(
				BUFFER_SIZE);

		private DataOutputStream probWriter = new DataOutputStream(byteBuffer);

		private long queryTime = 0;

		private long totalKeys = 0;

		private int noOfQueries = 0;

		private QueryRunnable(Socket querySocket) {
			this.querySocket = querySocket;
		}

		@Override
		public void run() {
			try {
				runWithExceptions();
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		}

		private void runWithExceptions() throws IOException {
			try (DataInputStream queryReader = new DataInputStream(
					new BufferedInputStream(querySocket.getInputStream()))) {
				try (OutputStream out = querySocket.getOutputStream()) {
					StopWatch stopWatch = new StopWatch();
					// A bit nasty, but will block on the readInt.
					// It's not really polling. Honest!
					try {
						int querySize = queryReader.readInt();
						totalKeys += querySize;
						stopWatch.start();
						for (int i = 0; i < querySize; ++i) {
							int provInt = queryReader.readInt();
							byte prov = (byte) provInt;
							int source = queryReader.readInt();
							int target = queryReader.readInt();
							if (model.containsKey(prov)
									&& model.get(prov).containsKey(source)
									&& model.get(prov).get(source)
											.containsKey(target)) {
								probWriter.writeDouble(model.get(prov)
										.get(source).get(target));
							} else {
								probWriter.writeDouble(Double.MAX_VALUE);
							}
						}
						byteBuffer.writeTo(out);
						byteBuffer.reset();
						stopWatch.stop();
						queryTime += stopWatch.getTime();
						if (++noOfQueries == 1000) {
							System.out.println("Time per key = "
									+ (double) queryTime / (double) totalKeys);
							noOfQueries = 0;
							queryTime = totalKeys = 0;
						}
					} catch (EOFException e) {
						System.out.println("Connection from mapper closed");
					}
				}
			}
			querySocket.close();
		}
	}

	static final String TTABLE_S2T_SERVER_PORT = "ttable_s2t_server_port";

	static final String TTABLE_T2S_SERVER_PORT = "ttable_t2s_server_port";

	private static final String LEX_TABLE_TEMPLATE = "ttable_server_template";

	private static final String GENRE = "$GENRE";

	private static final String DIRECTION = "$DIRECTION";

	private ServerSocket serverSocket;

	private Map<Byte, Map<Integer, Map<Integer, Double>>> model = new HashMap<>();

	private Runnable server = new Runnable() {

		@Override
		public void run() {
			while (true) {
				try {
					Socket querySocket = serverSocket.accept();
					threadPool.execute(new QueryRunnable(querySocket));
				} catch (SocketException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

		}
	};

	private void startServer() {
		Thread serverThread = new Thread(server);
		serverThread.setDaemon(true);
		serverThread.start();
	}

	private void loadModel(String modelFile, byte prov)
			throws FileNotFoundException, IOException {
		try (BufferedReader br = new BufferedReader(new InputStreamReader(
				new GZIPInputStream(new FileInputStream(modelFile))))) {
			String line;
			int count = 1;
			while ((line = br.readLine()) != null) {
				if (count % 1000000 == 0) {
					System.err.println("Processed " + count + " lines");
				}
				count++;
				line = line.replace("NULL", "0");
				String[] parts = StringUtils.split(line, '\\', ' ');
				try {
					int sourceWord = Integer.parseInt(parts[0]);
					int targetWord = Integer.parseInt(parts[1]);
					double model1Probability = Double.parseDouble(parts[2]);
					if (!model.get(prov).containsKey(sourceWord)) {
						model.get(prov).put(sourceWord,
								new HashMap<Integer, Double>());
					}
					model.get(prov).get(sourceWord)
							.put(targetWord, model1Probability);
				} catch (NumberFormatException e) {
					System.out.println("Unable to parse line: "
							+ e.getMessage() + "\n" + line);
				}
			}
		}
	}

	private void setup(Configuration conf, String direction,
			boolean source2Target) throws IOException, InterruptedException {
		int serverPort;
		if (source2Target) {
			serverPort = Integer.parseInt(conf.get(TTABLE_S2T_SERVER_PORT));
		} else {
			serverPort = Integer.parseInt(conf.get(TTABLE_T2S_SERVER_PORT));
		}
		serverSocket = new ServerSocket(serverPort);
		String lexTemplate = conf.get(LEX_TABLE_TEMPLATE);
		String allString = lexTemplate.replace(GENRE, "ALL").replace(DIRECTION,
				direction);
		System.out.println("Loading " + allString);
		String[] provenances = conf.getStrings(ProvenanceCountMap.PROV);
		ExecutorService loaderThreadPool = Executors.newFixedThreadPool(4);
		model.put((byte) 0, new HashMap<Integer, Map<Integer, Double>>());
		loaderThreadPool.execute(new LoadTask(allString, (byte) 0));
		for (int i = 0; i < provenances.length; ++i) {
			String provString = lexTemplate.replace(GENRE, provenances[i])
					.replace(DIRECTION, direction);
			System.out.println("Loading " + provString);
			byte prov = (byte) (i + 1);
			model.put(prov, new HashMap<Integer, Map<Integer, Double>>());
			loaderThreadPool.execute(new LoadTask(provString, prov));
		}
		loaderThreadPool.shutdown();
		loaderThreadPool.awaitTermination(3, TimeUnit.HOURS);
		System.gc();
	}

	@Override
	public void close() throws IOException {
		threadPool.shutdown();
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class TTableServerParameters {
		@Parameter(names = { "--ttable_s2t_server_port" }, description = "TTable source-to-target server port")
		public String ttable_s2t_server_port = "4949";

		@Parameter(names = { "--ttable_s2t_host" }, description = "TTable source-to-target host name")
		public String ttable_s2t_host = "localhost";

		@Parameter(names = { "--ttable_t2s_server_port" }, description = "TTable target-to-source server port")
		public String ttable_t2s_server_port = "9494";

		@Parameter(names = { "--ttable_t2s_host" }, description = "TTable target-to-source host name")
		public String ttable_t2s_host = "localhost";

		@Parameter(names = { "--ttable_server_template" }, description = "TTable target-to-source host name", required = true)
		public String ttable_server_template;

		@Parameter(names = { "--ttable_direction" }, description = "TTable direction for the lexical model ('s2t' or 't2s')", required = true)
		public String ttable_direction;

		@Parameter(names = { "--ttable_language_pair" }, description = "TTable language pair for the lexical model (e.g. 'en2ru' or 'ru2en')", required = true)
		public String ttable_language_pair;

		@Parameter(names = { "--provenance" }, description = "Comma-separated list of provenances", required = true)
		public String provenance;
	}

	public int run(String[] args) throws IllegalArgumentException,
			IllegalAccessException, IOException, InterruptedException {
		TTableServerParameters params = new TTableServerParameters();
		JCommander cmd = new JCommander(params);

		try {
			cmd.parse(args);
			Configuration conf = getConf();
			Util.ApplyConf(cmd, "", conf);
			boolean source2Target;
			if (params.ttable_direction.equals("s2t")) {
				source2Target = true;
			} else if (params.ttable_direction.equals("t2s")) {
				source2Target = false;
			} else {
				throw new RuntimeException("Unknown direction: " + args[2]);
			}
			try (TTableServer server = new TTableServer()) {
				server.setup(conf, params.ttable_language_pair, source2Target);
				server.startServer();
				System.err.println("TTable server ready on port: "
						+ server.serverSocket.getLocalPort());
				Thread.sleep(24 * 60 * 60 * 1000); // Sleep for 24 hours
			}
		} catch (ParameterException e) {
			System.err.println(e.getMessage());
			cmd.usage();
		}

		return 1;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new TTableServer(), args);
		System.exit(res);
	}
}
