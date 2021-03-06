// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/json/json_writer.h"
#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

TEST(JSONWriterTest, Writing) {
  // Test null
  Value* root = Value::CreateNullValue();
  std::string output_js;
  JSONWriter::Write(root, false, &output_js);
  ASSERT_EQ("null", output_js);
  delete root;

  // Test empty dict
  root = new DictionaryValue;
  JSONWriter::Write(root, false, &output_js);
  ASSERT_EQ("{}", output_js);
  delete root;

  // Test empty list
  root = new ListValue;
  JSONWriter::Write(root, false, &output_js);
  ASSERT_EQ("[]", output_js);
  delete root;

  // Test Real values should always have a decimal or an 'e'.
  root = Value::CreateDoubleValue(1.0);
  JSONWriter::Write(root, false, &output_js);
  ASSERT_EQ("1.0", output_js);
  delete root;

  // Test Real values in the the range (-1, 1) must have leading zeros
  root = Value::CreateDoubleValue(0.2);
  JSONWriter::Write(root, false, &output_js);
  ASSERT_EQ("0.2", output_js);
  delete root;

  // Test Real values in the the range (-1, 1) must have leading zeros
  root = Value::CreateDoubleValue(-0.8);
  JSONWriter::Write(root, false, &output_js);
  ASSERT_EQ("-0.8", output_js);
  delete root;

  // Writer unittests like empty list/dict nesting,
  // list list nesting, etc.
  DictionaryValue root_dict;
  ListValue* list = new ListValue;
  root_dict.Set("list", list);
  DictionaryValue* inner_dict = new DictionaryValue;
  list->Append(inner_dict);
  inner_dict->SetInteger("inner int", 10);
  ListValue* inner_list = new ListValue;
  list->Append(inner_list);
  list->Append(Value::CreateBooleanValue(true));

  // Test the pretty-printer.
  JSONWriter::Write(&root_dict, false, &output_js);
  ASSERT_EQ("{\"list\":[{\"inner int\":10},[],true]}", output_js);
  JSONWriter::Write(&root_dict, true, &output_js);
  // The pretty-printer uses a different newline style on Windows than on
  // other platforms.
#if defined(OS_WIN)
#define JSON_NEWLINE "\r\n"
#else
#define JSON_NEWLINE "\n"
#endif
  ASSERT_EQ("{" JSON_NEWLINE
            "   \"list\": [ {" JSON_NEWLINE
            "      \"inner int\": 10" JSON_NEWLINE
            "   }, [  ], true ]" JSON_NEWLINE
            "}" JSON_NEWLINE,
            output_js);
#undef JSON_NEWLINE

  // Test keys with periods
  DictionaryValue period_dict;
  period_dict.SetWithoutPathExpansion("a.b", Value::CreateIntegerValue(3));
  period_dict.SetWithoutPathExpansion("c", Value::CreateIntegerValue(2));
  DictionaryValue* period_dict2 = new DictionaryValue;
  period_dict2->SetWithoutPathExpansion("g.h.i.j",
                                        Value::CreateIntegerValue(1));
  period_dict.SetWithoutPathExpansion("d.e.f", period_dict2);
  JSONWriter::Write(&period_dict, false, &output_js);
  ASSERT_EQ("{\"a.b\":3,\"c\":2,\"d.e.f\":{\"g.h.i.j\":1}}", output_js);

  DictionaryValue period_dict3;
  period_dict3.Set("a.b", Value::CreateIntegerValue(2));
  period_dict3.SetWithoutPathExpansion("a.b", Value::CreateIntegerValue(1));
  JSONWriter::Write(&period_dict3, false, &output_js);
  ASSERT_EQ("{\"a\":{\"b\":2},\"a.b\":1}", output_js);

  // Test ignoring binary values.
  root = BinaryValue::CreateWithCopiedBuffer("asdf", 4);
  JSONWriter::WriteWithOptions(root, false,
                               JSONWriter::OPTIONS_OMIT_BINARY_VALUES,
                               &output_js);
  ASSERT_TRUE(output_js.empty());
  delete root;

  ListValue binary_list;
  binary_list.Append(Value::CreateIntegerValue(5));
  binary_list.Append(BinaryValue::CreateWithCopiedBuffer("asdf", 4));
  binary_list.Append(Value::CreateIntegerValue(2));
  JSONWriter::WriteWithOptions(&binary_list, false,
                               JSONWriter::OPTIONS_OMIT_BINARY_VALUES,
                               &output_js);
  ASSERT_EQ("[5,2]", output_js);

  DictionaryValue binary_dict;
  binary_dict.Set("a", Value::CreateIntegerValue(5));
  binary_dict.Set("b", BinaryValue::CreateWithCopiedBuffer("asdf", 4));
  binary_dict.Set("c", Value::CreateIntegerValue(2));
  JSONWriter::WriteWithOptions(&binary_dict, false,
                               JSONWriter::OPTIONS_OMIT_BINARY_VALUES,
                               &output_js);
  ASSERT_EQ("{\"a\":5,\"c\":2}", output_js);
}

}  // namespace base
