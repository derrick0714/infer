#ifndef DNS_PCAP_READER
#define DNS_PCAP_READER

#include <fstream>
#include <arpa/inet.h>

#include "../FileReader.hpp"
#include "PDXRecord.h"

namespace vn {
    namespace arl {
        namespace shared {

            template <typename FileEnumeratorType>
            class PDXFileReader : public FileReader <FileEnumeratorType, PDXRecord> {
            private:
                std::ifstream curFile;
                TimeStamp base_time;

            public:
                explicit PDXFileReader(const FileEnumeratorType& fEnum);

                ~PDXFileReader();

                virtual boost::shared_ptr <Serializable <PDXRecord> > read();
            };

            template <typename FileEnumeratorType>
            PDXFileReader<FileEnumeratorType>::PDXFileReader(const FileEnumeratorType& fEnum) :
            FileReader<FileEnumeratorType, PDXRecord>(fEnum), curFile(NULL) {
            }

            template <typename FileEnumeratorType>
            PDXFileReader<FileEnumeratorType>::~PDXFileReader() {
                if (curFile.is_open()) {
                    curFile.close();
                }
            }

            template <typename FileEnumeratorType>
            boost::shared_ptr<Serializable <PDXRecord> > PDXFileReader<FileEnumeratorType>::read() {
                using namespace std;
                using namespace boost;

                Serializable <PDXRecord>* newRecord;

                // open the next file to read.
                if (!curFile.is_open()) {
                    if (this->fIt == this->fEnum.end()) {
                        // Don't bother, no more files.
                        return shared_ptr<Serializable <PDXRecord> >();
                    }

                    const char* filename = (this->fIt)->file_string().c_str();
                    curFile.open(filename, ios::in | ios::binary);

                    if (curFile.fail()) {
                        this->_error = true;
                        this->_errorMsg = string(" Failed to open file ") + filename;

                        return shared_ptr<Serializable <PDXRecord> >();
                    }

                    arl_file_hdr_t header;

                    curFile.read((char*) (&header), sizeof (arl_file_hdr_t));

                    header.base_date = ntohl(header.base_date);

                    this->base_time.set(header.base_date, 0);
                    this->fIt++;
                }

                //do the reading.
                newRecord = new PDXRecord(&(this->base_time));

                char record[sizeof (pidx_data_t)];
                string dataStr;

                curFile.read(record, sizeof (record));

                if (curFile.eof()) {
                    curFile.close();

                    return this->read();
                } else {
                    newRecord->unserialize(dataStr.insert(0, record, sizeof (record)));
                }

                return shared_ptr<Serializable <PDXRecord> >(newRecord);
            }

        } // namespace shared
    } // namespace arl
} // namespace vn

#endif
