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

#include "yggdrasil_decision_forests/dataset/tensorflow_no_dep/tf_record.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "yggdrasil_decision_forests/dataset/tensorflow_no_dep/example.pb.h"
#include "yggdrasil_decision_forests/utils/filesystem.h"
#include "yggdrasil_decision_forests/utils/logging.h"
#include "yggdrasil_decision_forests/utils/test.h"
#include "yggdrasil_decision_forests/utils/testing_macros.h"

namespace yggdrasil_decision_forests::dataset::tensorflow_no_dep {

using test::EqualsProto;

std::string DatasetDir() {
  return file::JoinPath(test::DataRootDirectory(),
                        "yggdrasil_decision_forests/"
                        "test_data/dataset");
}

// Third toy example.
ydf_tensorflow::Example ThirdExample() {
  return PARSE_TEST_PROTO(
      R"pb(
        features {
          feature {
            key: "Bool_1"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "Bool_2"
            value {}
          }
          feature {
            key: "Cat_1"
            value { bytes_list { value: "C" } }
          }
          feature {
            key: "Cat_2"
            value {}
          }
          feature {
            key: "Cat_3"
            value { float_list { value: 3 } }
          }
          feature {
            key: "Cat_set_1"
            value { bytes_list { value: "x" value: "y" value: "z" } }
          }
          feature {
            key: "Cat_set_2"
            value { bytes_list { value: "z" value: "x" value: "y" } }
          }
          feature {
            key: "Num_1"
            value { float_list { value: 4 } }
          }
          feature {
            key: "Num_2"
            value { float_list { value: 4 } }
          }
        }
      )pb");
}

TEST(TFRecord, Reader) {
  ASSERT_OK_AND_ASSIGN(
      auto reader,
      TFRecordReader::Create(file::JoinPath(
          DatasetDir(), "toy.nocompress-tfe-tfrecord-00000-of-00002")));

  int message_idx = 0;
  while (true) {
    ydf_tensorflow::Example message;
    ASSERT_OK_AND_ASSIGN(const bool has_value, reader->Next(&message));
    if (!has_value) {
      break;
    }
    YDF_LOG(INFO) << message.DebugString();
    if (message_idx == 3) {
      EXPECT_THAT(message, EqualsProto(ThirdExample()));
    }
    message_idx++;
  }
  ASSERT_OK(reader->Close());
  EXPECT_EQ(message_idx, 3);
}

TEST(TFRecord, ShardedReader) {
  ShardedTFRecordReader<ydf_tensorflow::Example> reader;
  ASSERT_OK(reader.Open(
      file::JoinPath(DatasetDir(), "toy.nocompress-tfe-tfrecord@2")));

  int message_idx = 0;
  while (true) {
    ydf_tensorflow::Example message;
    ASSERT_OK_AND_ASSIGN(const bool has_value, reader.Next(&message));
    if (!has_value) {
      break;
    }
    YDF_LOG(INFO) << message.DebugString();
    if (message_idx == 3) {
      EXPECT_THAT(message, EqualsProto(ThirdExample()));
    }
    message_idx++;
  }
  EXPECT_EQ(message_idx, 4);
}

}  // namespace yggdrasil_decision_forests::dataset::tensorflow_no_dep