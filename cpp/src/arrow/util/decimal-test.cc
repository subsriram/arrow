// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//

#include "arrow/util/decimal.h"

#include "gtest/gtest.h"

#include "arrow/test-util.h"

namespace arrow {
namespace decimal {

template <typename T>
class DecimalTest : public ::testing::Test {
 public:
  DecimalTest() : string_value("234.23445") { integer_value.value = 23423445; }
  Decimal<T> integer_value;
  std::string string_value;
};

typedef ::testing::Types<int32_t, int64_t, int128_t> DecimalTypes;
TYPED_TEST_CASE(DecimalTest, DecimalTypes);

TYPED_TEST(DecimalTest, TestToString) {
  Decimal<TypeParam> decimal(this->integer_value);
  int precision = 8;
  int scale = 5;
  std::string result = ToString(decimal, precision, scale);
  ASSERT_EQ(result, this->string_value);
}

TYPED_TEST(DecimalTest, TestFromString) {
  Decimal<TypeParam> expected(this->integer_value);
  Decimal<TypeParam> result;
  int precision, scale;
  ASSERT_OK(FromString(this->string_value, &result, &precision, &scale));
  ASSERT_EQ(result.value, expected.value);
  ASSERT_EQ(precision, 8);
  ASSERT_EQ(scale, 5);
}

TEST(DecimalTest, TestStringToInt32) {
  int32_t value = 0;
  StringToInteger("123", "456", 1, &value);
  ASSERT_EQ(value, 123456);
}

TEST(DecimalTest, TestStringToInt64) {
  int64_t value = 0;
  StringToInteger("123456789", "456", -1, &value);
  ASSERT_EQ(value, -123456789456);
}

TEST(DecimalTest, TestStringToInt128) {
  int128_t value = 0;
  StringToInteger("123456789", "456789123", 1, &value);
  ASSERT_EQ(value, 123456789456789123);
}

TEST(DecimalTest, TestFromString128) {
  static const std::string string_value("-23049223942343532412");
  Decimal<int128_t> result(string_value);
  int128_t expected = -230492239423435324;
  ASSERT_EQ(result.value, expected * 100 - 12);

  // Sanity check that our number is actually using more than 64 bits
  ASSERT_NE(result.value, static_cast<int64_t>(result.value));
}

TEST(DecimalTest, TestFromDecimalString128) {
  static const std::string string_value("-23049223942343.532412");
  Decimal<int128_t> result(string_value);
  int128_t expected = -230492239423435324;
  ASSERT_EQ(result.value, expected * 100 - 12);

  // Sanity check that our number is actually using more than 64 bits
  ASSERT_NE(result.value, static_cast<int64_t>(result.value));
}

TEST(DecimalTest, TestDecimal32Precision) {
  auto min_precision = DecimalPrecision<int32_t>::minimum;
  auto max_precision = DecimalPrecision<int32_t>::maximum;
  ASSERT_EQ(min_precision, 1);
  ASSERT_EQ(max_precision, 9);
}

TEST(DecimalTest, TestDecimal64Precision) {
  auto min_precision = DecimalPrecision<int64_t>::minimum;
  auto max_precision = DecimalPrecision<int64_t>::maximum;
  ASSERT_EQ(min_precision, 10);
  ASSERT_EQ(max_precision, 18);
}

TEST(DecimalTest, TestDecimal128Precision) {
  auto min_precision = DecimalPrecision<int128_t>::minimum;
  auto max_precision = DecimalPrecision<int128_t>::maximum;
  ASSERT_EQ(min_precision, 19);
  ASSERT_EQ(max_precision, 38);
}

TEST(DecimalTest, TestDecimal32SignedRoundTrip) {
  Decimal32 expected(std::string("-3402692"));

  uint8_t stack_bytes[4] = {0};
  uint8_t* bytes = stack_bytes;
  ToBytes(expected, &bytes);

  Decimal32 result;
  FromBytes(bytes, &result);
  ASSERT_EQ(expected.value, result.value);
}

TEST(DecimalTest, TestDecimal64SignedRoundTrip) {
  Decimal64 expected(std::string("-34034293045.921"));

  uint8_t stack_bytes[8] = {0};
  uint8_t* bytes = stack_bytes;
  ToBytes(expected, &bytes);

  Decimal64 result;
  FromBytes(bytes, &result);

  ASSERT_EQ(expected.value, result.value);
}

TEST(DecimalTest, TestDecimal128StringAndBytesRoundTrip) {
  std::string string_value("-340282366920938463463374607431.711455");
  Decimal128 expected(string_value);

  std::string expected_string_value("-340282366920938463463374607431711455");
  int128_t expected_underlying_value(expected_string_value);

  ASSERT_EQ(expected.value, expected_underlying_value);

  uint8_t stack_bytes[16] = {0};
  uint8_t* bytes = stack_bytes;
  bool is_negative;
  ToBytes(expected, &bytes, &is_negative);

  ASSERT_TRUE(is_negative);

  Decimal128 result;
  FromBytes(bytes, is_negative, &result);

  ASSERT_EQ(expected.value, result.value);
}
}  // namespace decimal
}  // namespace arrow
