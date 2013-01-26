#ifndef NETFLOW_FILE_READER_HPP
#define NETFLOW_FILE_READER_HPP

#include <fstream>
#include <arpa/inet.h>

#include "../FileReader.hpp"
#include "NetFlowRecords.h"

namespace vn {
    namespace arl {
        namespace shared {

            template <typename FileEnumeratorType>
            class NetFlowARLReader : public FileReader <FileEnumeratorType, NetFlowARLRecord> {
            private:
                std::ifstream curFile;
                int fileVersion;

            public:
                explicit NetFlowARLReader(const FileEnumeratorType& fEnum);

                ~NetFlowARLReader();

                /// \brief if this method returns a NULL pointer but contains no errors
                ///        then there are no more available records.
                virtual boost::shared_ptr <Serializable <NetFlowARLRecord> > read();
            };

            template <typename FileEnumeratorType>
            NetFlowARLReader<FileEnumeratorType>::NetFlowARLReader(const FileEnumeratorType& fEnum) :
            FileReader<FileEnumeratorType, NetFlowARLRecord>(fEnum), curFile(NULL) {
            }

            template <typename FileEnumeratorType>
            NetFlowARLReader<FileEnumeratorType>::~NetFlowARLReader() {
                if (curFile.is_open()) {
                    curFile.close();
                }
            }

            template <typename FileEnumeratorType>
            boost::shared_ptr<Serializable <NetFlowARLRecord> > NetFlowARLReader<FileEnumeratorType>::read() {
                using namespace std;
                using namespace boost;

                Serializable <NetFlowARLRecord>* newRecord;

                // open the next file to read.
                if (!curFile.is_open()) {
                    if (this->fIt == this->fEnum.end()) {
                        // Don't bother, no more files.
                        return shared_ptr<Serializable <NetFlowARLRecord> >();
                    }

                    const char* filename = (this->fIt)->file_string().c_str();
                    curFile.open(filename, ios::in | ios::binary);

                    if (curFile.fail()) {
                        this->_error = true;
                        this->_errorMsg = string("Failed to open file ") + filename;

                        return shared_ptr<Serializable <NetFlowARLRecord> >();
                    }

                    arl_file_hdr_t header;

                    curFile.read((char*) (&header), 4);

                    this->fileVersion = ntohl(header.version);
                    this->fIt++;
                }

                //do the reading.
                switch(this->fileVersion) {
                    case vn::arl::shared::NetFlowARLRecord::NetFlow_v1:
                        newRecord = new NetFlowARL_v1_Record();
                        break;
                        
                    case vn::arl::shared::NetFlowARLRecord::NetFlow_v2:
                        newRecord = new NetFlowARL_v2_Record();
                        break;

                    case vn::arl::shared::NetFlowARLRecord::NetFlow_v3:
                        newRecord = new NetFlowARL_v3_Record();
                        break;

                    default: {
                        std::stringstream out;

                        out << " Bad version number: " << this->fileVersion;

                        this->_error = true;
                        this->_errorMsg = out.str();

                        return shared_ptr<Serializable <NetFlowARLRecord> >();
                    }
                }

                char record[sizeof (flow_v3_record_t)];
                string dataStr;

                curFile.read(record, newRecord->size());

                if (curFile.eof()) {
                    curFile.close();

                    // reached the end of file, so try reading the next record.
                    return this->read();
                } else {
                    newRecord->unserialize(dataStr.insert(0, record, sizeof (record)));
                }

                return shared_ptr<Serializable <NetFlowARLRecord> >(newRecord);
            }

        } // namespace shared
    } // namespace arl
} // namespace vn

#endif
