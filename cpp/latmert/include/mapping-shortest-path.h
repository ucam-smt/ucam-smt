// shortest-path.h

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: allauzen@google.com (Cyril Allauzen)
//
// \file
// Functions to find shortest paths in an FST.
//
// This file has been modified by Aurelien Waite from the University of
// Cambridge.
//
// We found that mapping a lattice from tropical monomial weights to
// tropical weights was very expensive. This version of shortest path
// maps the weights on the fly with a value of gamma.

#ifndef MERT_FST_LIB_SHORTEST_PATH_H__
#define MERT_FST_LIB_SHORTEST_PATH_H__

#include <functional>
#include <utility>
#include <vector>

#include <fst/cache.h>
#include <fst/determinize.h>
#include <fst/queue.h>
#include <fst/shortest-distance.h>
#include <fst/test-properties.h>

namespace mertfst {

// Shortest-path algorithm: normally not called directly; prefer
// 'ShortestPath' below with n=1. 'ofst' contains the shortest path in
// 'ifst'. 'distance' returns the shortest distances from the source
// state to each state in 'ifst'. 'opts' is used to specify options
// such as the queue discipline, the arc filter and delta.
//
// The shortest path is the lowest weight path w.r.t. the natural
// semiring order.
//
// The weights need to be right distributive and have the path (kPath)
// property.
template<class FArc, class TArc>
void SingleShortestPath(const fst::Fst<FArc> &ifst, fst::MutableFst<TArc> *ofst, F gamma) {
	typedef typename FArc::StateId StateId;
	typedef typename FArc::Weight FWeight;
	typedef typename TArc::Weight Weight;

	std::vector<Weight> distance;
	fst::AnyArcFilter < TArc > arc_filter;
	fst::AutoQueue<typename FArc::StateId> state_queue(*ofst, &distance, arc_filter);

	ofst->DeleteStates();
	ofst->SetInputSymbols(ifst.InputSymbols());
	ofst->SetOutputSymbols(ifst.OutputSymbols());

	if (ifst.Start() == fst::kNoStateId)
		return;

	std::vector<Weight> rdistance;
	std::vector<bool> enqueued;
	std::vector<StateId> parent;
	std::vector<TArc> arc_parent;

	//Queue *state_queue = opts.state_queue;
	StateId source =  ifst.Start();
	Weight f_distance = Weight::Zero();
	StateId f_parent = fst::kNoStateId;

	distance.clear();
	state_queue.Clear();

	while (distance.size() < source) {
		distance.push_back(Weight::Zero());
		enqueued.push_back(false);
		parent.push_back(fst::kNoStateId);
		arc_parent.push_back(TArc(fst::kNoLabel, fst::kNoLabel, Weight::Zero(),
				fst::kNoStateId));
	}
	distance.push_back(Weight::One());
	parent.push_back(fst::kNoStateId);
	arc_parent.push_back(TArc(fst::kNoLabel, fst::kNoLabel, Weight::Zero(), fst::kNoStateId));
	state_queue.Enqueue(source);
	enqueued.push_back(true);

	while (!state_queue.Empty()) {
		StateId s = state_queue.Head();
		state_queue.Dequeue();
		enqueued[s] = false;
		Weight sd = distance[s];
		if (ifst.Final(s) != FWeight::Zero()) {
			Weight w = Times(sd, ifst.Final(s).Map(gamma));
			if (f_distance != Plus(f_distance, w)) {
				f_distance = Plus(f_distance, w);
				f_parent = s;
			}
		}
		for (fst::ArcIterator < fst::Fst<FArc> > aiter(ifst, s); !aiter.Done(); aiter.Next()) {
			const FArc &arc = aiter.Value();
			while (distance.size() <= arc.nextstate) {
				distance.push_back(Weight::Zero());
				enqueued.push_back(false);
				parent.push_back(fst::kNoStateId);
				arc_parent.push_back(TArc(fst::kNoLabel, fst::kNoLabel, Weight::Zero(),
						fst::kNoStateId));
			}
			Weight &nd = distance[arc.nextstate];
			Weight mw = arc.weight.Map(gamma);
			Weight w = Times(sd, mw);
			if (nd != Plus(nd, w)) {
				nd = Plus(nd, w);
				parent[arc.nextstate] = s;
				TArc tarc(arc.ilabel, arc.olabel, mw, arc.nextstate);
				arc_parent[arc.nextstate] = tarc;
				if (!enqueued[arc.nextstate]) {
					state_queue.Enqueue(tarc.nextstate);
					enqueued[arc.nextstate] = true;
				} else {
					state_queue.Update(tarc.nextstate);
				}
			}
		}
	}
	distance[source] = Weight::One();
	parent[source] = fst::kNoStateId;

	StateId s_p = fst::kNoStateId, d_p = fst::kNoStateId;
	for (StateId s = f_parent, d = fst::kNoStateId; s != fst::kNoStateId; d = s, s
			= parent[s]) {
		d_p = s_p;
		s_p = ofst->AddState();
		if (d == fst::kNoStateId) {
			ofst->SetFinal(s_p, ifst.Final(f_parent).Map(gamma));
		} else {
			arc_parent[d].nextstate = d_p;
			ofst->AddArc(s_p, arc_parent[d]);
		}
	}
	ofst->SetStart(s_p);
}

} // namespace mertfst

#endif  // MERT_FST_LIB_SHORTEST_PATH_H__
