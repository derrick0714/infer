#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include <boost/shared_ptr.hpp>

#include "FileEnumerator.hpp"
#include "Serializable.hpp"

namespace vn {
    namespace arl {
        namespace shared {

            /// \class FileReader FileReader.hpp
            /// \brief Abstract base class for a FileReader
            ///
            /// A FileReader class implements/inherits capabilities to read a specific type
            /// of data from a specific type of files. A FileReader makes the semantics of
            /// reading a file and organization of data within a file transparent to its
            /// users.  Any FileReader must inherit from this class.
            ///
            /// \sa FileEnumerator
            ///

            template <typename FileEnumeratorType, typename ReadDataType>
            class FileReader {
            public:
                /// \brief Constructor
                /// \param fEnum The FileEnumerator from which to obtain the files to read
                ///
                /// This constructor shall be called in the initialization list of the
                /// constructor of any derived class. It initializes fEnum, fIt, and
                /// isInitialized.

                explicit FileReader(const FileEnumeratorType &fEnum)
                : fEnum(fEnum),
                fIt(this -> fEnum . begin()),
                _error(!(this -> fEnum)),
                _errorMsg(_error ? "FileEnumerator not initialized" : "") {
                }

                /// \brief Virtual destructor

                virtual ~FileReader() {
                }

                /// \brief Read data
                /// \returns a boost::shared_ptr pointing to the data read.
                ///
                /// If the read failed, or if there is nothing left to read, the
                /// returned boost::shared_ptr is empty.
                virtual boost::shared_ptr <Serializable <ReadDataType> > read() = 0;

                /// \brief Get the error string
                /// \returns the error string.

                const std::string & error() const {
                    return _errorMsg;
                }

                /// \brief Boolean cast operator
                /// \returns true if there is no error
                ///
                /// This is the means by which clients will test for an error.

                operator bool() const {
                    return !_error;
                }

            protected:
                /// The FileEnumerator from which to obtain the files to read
                const FileEnumeratorType fEnum;

                /// The const_iterator used to iterate through the files to read
                typename FileEnumeratorType::const_iterator fIt;

                /// The error status
                bool _error;

                /// The error message
                std::string _errorMsg;
            };

        } // namespace shared
    } // namespace arl
} // namespace vn

#endif
