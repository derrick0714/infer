#ifndef INFER_LIB_ANALYSIS_HOST_EXPOSURE_HOST_AS_EXPOSURE_DATUM_HPP_
#define INFER_LIB_ANALYSIS_HOST_EXPOSURE_HOST_AS_EXPOSURE_DATUM_HPP_

struct host_as_exposure_datum {
	uint32_t host;
	uint16_t asn;
	size_t ingress_bytes;
	size_t egress_bytes;
};

#endif
