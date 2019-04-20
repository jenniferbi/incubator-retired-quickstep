/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 **/

#include "histogram/HTree.hpp"
#include "types/TypedValue.hpp"

#include "histogram/HTree.pb.h"

namespace quickstep {

using std::vector;
using std::shared_ptr;
using std::make_shared;

template<typename T>
htree_element<T> htree_element<T>::ReconstructFromProto(const
				serialization::HTree_HTreeElem &proto) {
	LOG_WARNING("deserialization for non-HypedV HTree not supported")
}

template<typename T>
htree_node<T>* htree_node<T>::ReconstructFromProto(const 
				serialization::HTree_HTreeNode &proto) {
	LOG_WARNING("deserialization for non-HypedV HTree not supported")
}

// helpers in charge of allocating memory via new
// caller creates shared ptr from naked ptr
template<>
htree_element<HypedValue> htree_element<HypedValue>::ReconstructFromProto(
					const serialization::HTree_HTreeElem &proto) {

	interval<HypedValue>* key_ = nullptr;

	if (proto.has_key()) {	
		const serialization::HTree_HTreeInterval &key_proto = proto.key();
		bool has_min_ = false, has_max_ = false;
		HypedValue* max_ = nullptr, *min_ = nullptr;
		if (key_proto.has_low()) {
			has_min_ = true;
			*min_ = HypedValue(
				TypedValue::ReconstructFromProto(key_proto.low()));
		}
		if (key_proto.has_high()) {
			has_max_ = true;
			*max_ = HypedValue(
				TypedValue::ReconstructFromProto(key_proto.high()));
		}
		*key_ = interval<HypedValue>(has_min_, *min_, has_max_, *max_);
	}

	if (proto.has_child()) {
		shared_ptr<htree_node<HypedValue> > child_(
			htree_node<HypedValue>::ReconstructFromProto(proto.child()));
		return htree_element<HypedValue>(*key_, child_);
	} 

	shared_ptr<bucket<HypedValue> > bkt_entry;
	vector<interval<HypedValue> > dims;
	dims.reserve(proto.bucket_size());
	for (int bucket_num = 0; bucket_num < proto.bucket_size(); ++bucket_num) {
		const serialization::HTree_HTreeInterval &bkt_proto
								 = proto.bucket(bucket_num);
		bool has_min_ = false, has_max_ = false;
		HypedValue* max_ = nullptr, *min_ = nullptr;
		if (bkt_proto.has_low()) {
			has_min_ = true;
			*min_ = HypedValue(
				TypedValue::ReconstructFromProto(bkt_proto.low()));
		}
		if (bkt_proto.has_high()) {
			has_max_ = true;
			*max_ = HypedValue(
				TypedValue::ReconstructFromProto(bkt_proto.high()));
		}
		dims.emplace_back(has_min_, *min_, has_max_, *max_);
	} // end for
	bkt_entry = make_shared<bucket<HypedValue> >(dims);
	return htree_element<HypedValue>(*key_, bkt_entry);

}

template<>
htree_node<HypedValue>* htree_node<HypedValue>::ReconstructFromProto(const 
				serialization::HTree_HTreeNode &proto) {
	vector<htree_element<HypedValue> > elts;
	elts.reserve(proto.elements_size());	
	for (int elt_num = 0; elt_num < proto.elements_size(); ++elt_num) {
		const serialization::HTree_HTreeElem &elt = proto.elements(elt_num);
		elts.push_back(
	    	htree_element<HypedValue>::ReconstructFromProto(elt));	
	}
	return new htree_node<HypedValue>(1, elts);
}


HTree::HTree(
    const serialization::HTree &proto) {

  if (proto.has_root()) {
    root_.reset(htree_node<HypedValue>::ReconstructFromProto(proto.root()));
  }

}	

template<typename T>
void htree_element<T>::getProtoHelper(
				serialization::HTree_HTreeElem &proto,
				const htree_element<T>& elem) {
	LOG_WARNING("serialization for non-HypedV HTree not supported")
}

template<typename T>
void htree_node<T>::getProtoHelper(
				serialization::HTree_HTreeNode &proto,
				const htree_node<T>& node) {
	LOG_WARNING("serialization for non-HypedV HTree not supported")
}

template <>
void htree_element<HypedValue>::getProtoHelper(
				serialization::HTree_HTreeElem &proto,
				const htree_element<HypedValue>& elem) {
	const interval<HypedValue>& mykey = elem.key;
	serialization::HTree_HTreeInterval* invl_proto = proto.mutable_key();
	if (mykey.has_min) {
		invl_proto->mutable_low()->CopyFrom(
				(mykey.min.getTypedValue()).getProto());
	}
	if (mykey.has_max) {
		invl_proto->mutable_high()->CopyFrom(
				(mykey.max.getTypedValue()).getProto());
	}
	if (elem.child != nullptr) {
		serialization::HTree_HTreeNode* node_proto = proto.mutable_child();
		htree_node<HypedValue>::getProtoHelper(*node_proto, *(elem.child));
				
	} else if (elem.bkt != nullptr) {
		for (const auto &invl : elem.bkt->get_dimensions()) {
			serialization::HTree_HTreeInterval* bkt_proto = proto.add_bucket();
			if (invl.has_min) {
				bkt_proto->mutable_low()->CopyFrom(
						(invl.min.getTypedValue()).getProto());
			}
			if (invl.has_max) {
				bkt_proto->mutable_high()->CopyFrom(
						(invl.max.getTypedValue()).getProto());
			}
		} // end for thru intervals
	} // end bucket case
	
}

template <>
void htree_node<HypedValue>::getProtoHelper(
    serialization::HTree_HTreeNode &proto, const htree_node<HypedValue>& node) {
	for (const auto &elt : node.elements) {
		serialization::HTree_HTreeElem* elem_proto= proto.add_elements();
		htree_element<HypedValue>::getProtoHelper(*elem_proto, elt);
	}

}

serialization::HTree HTree::getProto() const {

  serialization::HTree proto;

  serialization::HTree_HTreeNode* node_proto = proto.mutable_root();
  htree_node<HypedValue>::getProtoHelper(*node_proto, *root_);

  return proto;

}

}  // namespace quickstep
