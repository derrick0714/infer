#ifndef INFER_LIB_ANALYSIS_BANDWIDTH_CONTENT_BANDWIDTH_CONTENT_DATUM_HPP_
#define INFER_LIB_ANALYSIS_BANDWIDTH_CONTENT_BANDWIDTH_CONTENT_DATUM_HPP_

struct bandwidth_content_datum {
	uint32_t time_seconds;
	double ingress_bytes_per_second_plaintext;
	double ingress_bytes_per_second_bmp_image;
	double ingress_bytes_per_second_wav_audio;
	double ingress_bytes_per_second_compressed;
	double ingress_bytes_per_second_jpeg_image;
	double ingress_bytes_per_second_mp3_audio;
	double ingress_bytes_per_second_mpeg_video;
	double ingress_bytes_per_second_encrypted;
	double ingress_bytes_per_second_unknown;
	double egress_bytes_per_second_plaintext;
	double egress_bytes_per_second_bmp_image;
	double egress_bytes_per_second_wav_audio;
	double egress_bytes_per_second_compressed;
	double egress_bytes_per_second_jpeg_image;
	double egress_bytes_per_second_mp3_audio;
	double egress_bytes_per_second_mpeg_video;
	double egress_bytes_per_second_encrypted;
	double egress_bytes_per_second_unknown;
};

#endif