/*
 * Copyright 2022 Google LLC.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// A BlogSequence (BS) is a sequence of blobs (list of bytes) stored in a byte
// stream (e.g. a file).
//
// Writing usage example:
//
//   auto output_stream = file::OpenOutputFile(path).value();
//   auto writer = blob_sequence::Writer::Create(output_stream.get()).value();
//   CHECK_OK(writer.Write("HELLO"));
//   CHECK_OK(writer.Write("WORLD"));
//   CHECK_OK(writer.Close());
//   CHECK_OK(output_stream->Close());
//
// Reading usage example:
//
//   auto input_stream = file::OpenInputFile(path).value();
//   auto reader = blob_sequence::Reader::Create(input_stream.get()).value();
//   std::string blob;
//   CHECK(reader.Read(&blob).value());
//   CHECK(reader.Read(&blob).value());
//   CHECK_OK(reader.Close());
//   CHECK_OK(input_stream->Close());
//
#ifndef YGGDRASIL_DECISION_FORESTS_UTILS_BLOB_SEQUENCE_H_
#define YGGDRASIL_DECISION_FORESTS_UTILS_BLOB_SEQUENCE_H_

#include <cstdint>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "yggdrasil_decision_forests/utils/bytestream.h"
#include "yggdrasil_decision_forests/utils/zlib.h"

namespace yggdrasil_decision_forests {
namespace utils {
namespace blob_sequence {

enum class Compression : uint8_t {
  kNone = 0,
  kGZIP = 1,
};

// Blog sequence reader.
class Reader {
 public:
  // Creates a reader attached to a stream. Does not take ownership of "stream".
  static absl::StatusOr<Reader> Create(utils::InputByteStream* stream);

  // Creates a non attached reader.
  Reader() {}

  // Reads the next blob. Return false iff no more blobs are available.
  absl::StatusOr<bool> Read(std::string* blob);

  // Closes the reader. Does not close the stream (passed in the constructor)
  // Should be called BEFORE the stream is closed (if the stream has the concept
  // of being closed).
  absl::Status Close();

 private:
  InputByteStream& stream() {
    return gzip_stream_ ? *gzip_stream_ : *raw_stream_;
  }

  // Non-owned input stream.
  InputByteStream* raw_stream_ = nullptr;
  // gzip decoder is the file is compressed.
  std::unique_ptr<utils::GZipInputByteStream> gzip_stream_;
  uint16_t version_;
  Compression compression_;
};

// Blog sequence writer.
class Writer {
 public:
  // Creates a writer attached to a stream.  Does not take ownership of
  // "stream".
  static absl::StatusOr<Writer> Create(
      utils::OutputByteStream* stream,
      Compression compression = Compression::kNone);

  // Creates a non attached writer.
  Writer() {}

  // Writes a blob.
  absl::Status Write(absl::string_view blob);

  // Closes the writer. Does not close the stream passed in the constructor.
  // Should be called BEFORE the stream is closed (if the stream has the concept
  // of being closed).
  absl::Status Close();

 private:
  OutputByteStream& stream() {
    return gzip_stream_ ? *gzip_stream_ : *raw_stream_;
  }

  // Non-owned output stream.
  OutputByteStream* raw_stream_ = nullptr;
  // gzip encoder is the file is compressed.
  std::unique_ptr<utils::GZipOutputByteStream> gzip_stream_;
};

namespace internal {

// File header.
// Integer are stored in little endian.
struct FileHeader {
  // Should be 'BS';
  uint8_t magic[2];

  // Version of the format.
  // Version:
  //   0: Initial version.
  //   1: Add support for gzip compression.
  uint16_t version;

  // Compression.
  uint8_t compression;

  // Reserved until used (instead of creating a per-version header).
  // Should remain zero until used.
  uint8_t reserved2 = 0;
  uint16_t reserved1 = 0;
};

// Record header.
// Integer are stored in little endian.
struct RecordHeader {
  // Size of the record in bytes, excluding the header.
  uint32_t length;
};

};  // namespace internal

}  // namespace blob_sequence
}  // namespace utils
}  // namespace yggdrasil_decision_forests

#endif  // YGGDRASIL_DECISION_FORESTS_UTILS_BLOB_SEQUENCE_H_
