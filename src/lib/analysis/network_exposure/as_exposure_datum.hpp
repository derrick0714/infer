#ifndef INFER_LIB_ANALYSIS_BANDWIDTH_UTILIZATION_AS_EXPOSURE_DATUM_HPP_
#define INFER_LIB_ANALYSIS_BANDWIDTH_UTILIZATION_AS_EXPOSURE_DATUM_HPP_

struct as_exposure_datum {
	uint16_t asn;
	size_t internal_hosts_contacted;
	size_t ingress_bytes;
	size_t egress_bytes;
};

#endif
