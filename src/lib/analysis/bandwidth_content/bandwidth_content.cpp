#include <string>

#include "PostgreSQLBulkWriter.hpp"
#include "modules.hpp"
#include "hostPair.hpp"

#include "bandwidth_content_datum.hpp"

using namespace std;
using namespace tr1;

SharedState *_shared_state_p;

// stats collection
struct bandwidth_content_t {
	bandwidth_content_t()
		:breakdown(FlowStats::CONTENT_TYPES + 1, 0)
	{}

	bandwidth_content_t & operator+= (const bandwidth_content_t &b) {
		for (size_t i(0); i < breakdown.size(); ++i) {
			breakdown[i] += b.breakdown[i];
		}

		return *this;
	}

	vector<double> breakdown;
};

// key: interval_begin, data: content_utilization
map<uint32_t, bandwidth_content_t> _ingress_bw_content;
map<uint32_t, bandwidth_content_t> _egress_bw_content;

// configuration options
size_t _interval_length;
string _stats_schema;
string _bandwidth_content_stats_table;

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		_shared_state_p = &sharedState;

		if (moduleState.conf.get(_interval_length,
								 "interval-length",
								 "analysis_bandwidth_content")
				!= configuration::OK)
		{
			cerr << "analysis_bandwidth_content: "
						"missing or invalid interval-length"
				 << endl;
			return false;
		}

		if (moduleState.conf.get(_stats_schema,
								 "stats-schema",
								 "analysis_bandwidth_content")
				!= configuration::OK)
		{
			cerr << "analysis_bandwidth_content: "
						"missing or invalid stats-schema"
				 << endl;
			return false;
		}

		if (moduleState.conf.get(_bandwidth_content_stats_table,
								 "bandwidth-content-stats-table",
								 "analysis_bandwidth_content")
				!= configuration::OK)
		{
			cerr << "analysis_bandwidth_content: "
						"missing or invalid bandwidth-content-stats-table"
				 << endl;
			return false;
		}

		return true;
	}

	void aggregate(FlowStats *flowStats, size_t &) {
		static uint32_t begin;
		static uint32_t end;
		static bandwidth_content_t content_bytes;
		static map<uint32_t, bandwidth_content_t> *bw;

		if (isInternal(flowStats->sourceIP(),
					   *(_shared_state_p->localNetworks)))
		{
			if (isInternal(flowStats->destinationIP(),
						   *(_shared_state_p->localNetworks)))
			{
				return;
			}

			bw = &_egress_bw_content;
		}
		else {
			if (!isInternal(flowStats->destinationIP(),
							*(_shared_state_p->localNetworks)))
			{
				return;
			}

			bw = &_ingress_bw_content;
		}

		begin = flowStats->startTime().seconds() -
					flowStats->startTime().seconds() % _interval_length;
		end = flowStats->endTime().seconds() -
					(flowStats->endTime().seconds() % _interval_length) +
					_interval_length;
		content_bytes.breakdown[FlowStats::CONTENT_TYPES] =
			flowStats->numBytes();
		// FIXME this is annoying...solution?
		//for (size_t i(0); i < FlowStats::CONTENT_TYPES; ++i) {
		{
			FlowStats::ContentType i;

			i = FlowStats::PLAINTEXT_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::BMP_IMAGE_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::WAV_AUDIO_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::COMPRESSED_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::JPEG_IMAGE_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::MP3_AUDIO_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::MPEG_VIDEO_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);

			i = FlowStats::ENCRYPTED_TYPE;
			content_bytes.breakdown[i] = (flowStats->content(i) << 14);
			content_bytes.breakdown[FlowStats::CONTENT_TYPES] -=
				content_bytes.breakdown[i];
			content_bytes.breakdown[i] /= ((end - begin) / _interval_length);
		}
		content_bytes.breakdown[FlowStats::CONTENT_TYPES] /=
			((end - begin) / _interval_length);

		while (begin < end) {
			(*bw)[begin] += content_bytes;
			begin += _interval_length;
		}
	}

	int commit(PostgreSQLConnection &pg_conn,
			   size_t &,
			   const char *date)
	{
		struct tm tm_;
		if (strptime(date, "%Y-%m-%d", &tm_) == NULL) {
			cerr << "analysis_bandwidth_content: invalid date: " << date
				 << endl;
			return -1;
		}

		struct tm tm_begin, tm_end;
		memset(&tm_begin, 0, sizeof(tm_begin));
		memset(&tm_end, 0, sizeof(tm_end));

		tm_begin.tm_isdst = tm_end.tm_isdst = -1;
		tm_begin.tm_year = tm_end.tm_year = tm_.tm_year;
		tm_begin.tm_mon = tm_end.tm_mon = tm_.tm_mon;
		tm_begin.tm_mday = tm_end.tm_mday = tm_.tm_mday;
		tm_begin.tm_hour = 0;
		tm_end.tm_hour = 24;

		uint32_t begin(mktime(&tm_begin));
		uint32_t end(mktime(&tm_end));

		PostgreSQLBulkWriter<bandwidth_content_datum>
			writer(pg_conn,
				   _stats_schema,
				   _bandwidth_content_stats_table);

		bandwidth_content_datum datum;
		int rows;
		for (uint32_t i(begin); i < end; i += _interval_length) {
			datum.time_seconds = i;
			datum.ingress_bytes_per_second_plaintext =
				_ingress_bw_content[i]
					.breakdown[FlowStats::PLAINTEXT_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_bmp_image =
				_ingress_bw_content[i]
					.breakdown[FlowStats::BMP_IMAGE_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_wav_audio =
				_ingress_bw_content[i]
					.breakdown[FlowStats::WAV_AUDIO_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_compressed =
				_ingress_bw_content[i]
					.breakdown[FlowStats::COMPRESSED_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_jpeg_image =
				_ingress_bw_content[i]
					.breakdown[FlowStats::JPEG_IMAGE_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_mp3_audio =
				_ingress_bw_content[i]
					.breakdown[FlowStats::MP3_AUDIO_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_mpeg_video =
				_ingress_bw_content[i]
					.breakdown[FlowStats::MPEG_VIDEO_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_encrypted =
				_ingress_bw_content[i]
					.breakdown[FlowStats::ENCRYPTED_TYPE] / _interval_length;
			datum.ingress_bytes_per_second_unknown =
				_ingress_bw_content[i]
					.breakdown[FlowStats::CONTENT_TYPES] / _interval_length;
			datum.egress_bytes_per_second_plaintext =
				_egress_bw_content[i]
					.breakdown[FlowStats::PLAINTEXT_TYPE] / _interval_length;
			datum.egress_bytes_per_second_bmp_image =
				_egress_bw_content[i]
					.breakdown[FlowStats::BMP_IMAGE_TYPE] / _interval_length;
			datum.egress_bytes_per_second_wav_audio =
				_egress_bw_content[i]
					.breakdown[FlowStats::WAV_AUDIO_TYPE] / _interval_length;
			datum.egress_bytes_per_second_compressed =
				_egress_bw_content[i]
					.breakdown[FlowStats::COMPRESSED_TYPE] / _interval_length;
			datum.egress_bytes_per_second_jpeg_image =
				_egress_bw_content[i]
					.breakdown[FlowStats::JPEG_IMAGE_TYPE] / _interval_length;
			datum.egress_bytes_per_second_mp3_audio =
				_egress_bw_content[i]
					.breakdown[FlowStats::MP3_AUDIO_TYPE] / _interval_length;
			datum.egress_bytes_per_second_mpeg_video =
				_egress_bw_content[i]
					.breakdown[FlowStats::MPEG_VIDEO_TYPE] / _interval_length;
			datum.egress_bytes_per_second_encrypted =
				_egress_bw_content[i]
					.breakdown[FlowStats::ENCRYPTED_TYPE] / _interval_length;
			datum.egress_bytes_per_second_unknown =
				_egress_bw_content[i]
					.breakdown[FlowStats::CONTENT_TYPES] / _interval_length;

			writer.write(datum);

			++rows;
		}

		writer.flush();
		writer.close();

		return rows;
	}
}
