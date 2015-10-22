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
import org.apache.hadoop.util.StringUtils;

import uk.ac.cam.eng.extraction.hadoop.util.Util;
import uk.ac.cam.eng.util.CLI;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class TTableServer implements Closeable {

	final static int BUFFER_SIZE = 65536;

	private static final String GENRE = "$GENRE";

	private static final String DIRECTION = "$DIRECTION";

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

	private ServerSocket serverSocket;

	private Map<Byte, Map<Integer, Map<Integer, Double>>> model = new HashMap<>();

	private double minLexProb = 0;

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

	public void startServer() {
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
					if (model1Probability < minLexProb) {
						continue;
					}
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

	public void setup(CLI.TTableServerParameters params) throws IOException,
			InterruptedException {
		boolean source2Target;
		if (params.ttableDirection.equals("s2t")) {
			source2Target = true;
		} else if (params.ttableDirection.equals("t2s")) {
			source2Target = false;
		} else {
			throw new RuntimeException("Unknown direction: "
					+ params.ttableDirection);
		}
		int serverPort;
		if (source2Target) {
			serverPort = params.sp.ttableS2TServerPort;
		} else {
			serverPort = params.sp.ttableT2SServerPort;
			;
		}
		minLexProb = params.minLexProb;
		serverSocket = new ServerSocket(serverPort);
		String lexTemplate = params.ttableServerTemplate;
		String allString = lexTemplate.replace(GENRE, "ALL").replace(DIRECTION,
				params.ttableLanguagePair);
		System.out.println("Loading " + allString);
		String[] provenances = params.prov.provenance.split(",");
		ExecutorService loaderThreadPool = Executors.newFixedThreadPool(4);
		model.put((byte) 0, new HashMap<Integer, Map<Integer, Double>>());
		loaderThreadPool.execute(new LoadTask(allString, (byte) 0));
		for (int i = 0; i < provenances.length; ++i) {
			String provString = lexTemplate.replace(GENRE, provenances[i])
					.replace(DIRECTION, params.ttableLanguagePair);
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

	public static void main(String[] args) throws IllegalArgumentException,
			IllegalAccessException, IOException, InterruptedException {
		CLI.TTableServerParameters params = new CLI.TTableServerParameters();
		Util.parseCommandLine(args, params);

		try (TTableServer server = new TTableServer()) {
			server.setup(params);
			server.startServer();
			System.err.println("TTable server ready on port: "
					+ server.serverSocket.getLocalPort());
			Thread.sleep(24 * 60 * 60 * 1000); // Sleep for 24 hours
		}
	}

}
