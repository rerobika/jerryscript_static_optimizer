/* Copyright (c) 2020 Robert Fancsik
 *
 * Licensed under the BSD 3-Clause License
 * <LICENSE or https://opensource.org/licenses/BSD-3-Clause>.
 * This file may not be copied, modified, or distributed except
 * according to those terms.
 */

#ifndef BYTECODE_H
#define BYTECODE_H

#include "common.h"
#include "stack.h"

extern "C" {
#define this this_value
#include "ecma-function-object.h"
#undef this
}

namespace optimizer {

class BytecodeFlags {
public:
  BytecodeFlags(){};

  auto flags() const { return flags_; }
  void setFlags(uint16_t flags) { flags_ = flags; }

  bool uint16Arguments() {
    return (flags_ & CBC_CODE_FLAGS_UINT16_ARGUMENTS) != 0;
  }

  bool fullLiteralEncoding() {
    return (flags_ & CBC_CODE_FLAGS_FULL_LITERAL_ENCODING) != 0;
  }

  bool mappedArgumentsNeeded() {
    return (flags_ & CBC_CODE_FLAGS_MAPPED_ARGUMENTS_NEEDED) != 0;
  }

private:
  uint16_t flags_;
};

class BytecodeArguments {
public:
  BytecodeArguments()
      : argument_end_(0), register_end_(0), ident_end_(0),
        const_literal_end_(0), literal_end_(0) {}

  void setArguments(cbc_uint16_arguments_t *args) {
    argument_end_ = args->argument_end;
    register_end_ = args->register_end;
    ident_end_ = args->ident_end;
    const_literal_end_ = args->const_literal_end;
    literal_end_ = args->literal_end;
    size_ = static_cast<uint16_t>(sizeof(cbc_uint16_arguments_t));
  }

  void setArguments(cbc_uint8_arguments_t *args) {
    argument_end_ = static_cast<uint16_t>(args->argument_end);
    register_end_ = static_cast<uint16_t>(args->register_end);
    ident_end_ = static_cast<uint16_t>(args->ident_end);
    const_literal_end_ = static_cast<uint16_t>(args->const_literal_end);
    literal_end_ = static_cast<uint16_t>(args->literal_end);
    size_ = static_cast<uint16_t>(sizeof(cbc_uint8_arguments_t));
  }

  void setEncoding(uint16_t limit, uint16_t delta) {
    encoding_limit_ = limit;
    encoding_delta_ = delta;
  }

  auto registerCount() { return register_end_ - argument_end_; }
  auto literalCount() { return literal_end_ - register_end_; }

  auto argumentEnd() const { return argument_end_; }
  auto registerEnd() const { return register_end_; }
  auto identEnd() const { return ident_end_; }
  auto constLiteralEnd() const { return const_literal_end_; }
  auto literalEnd() const { return literal_end_; }
  auto encodingLimit() const { return encoding_limit_; }
  auto encodingDelta() const { return encoding_delta_; }
  auto size() const { return size_; }

private:
  uint16_t argument_end_;
  uint16_t register_end_;
  uint16_t ident_end_;
  uint16_t const_literal_end_;
  uint16_t literal_end_;
  uint16_t encoding_limit_;
  uint16_t encoding_delta_;
  uint16_t size_;
};

class LiteralPool {
public:
  LiteralPool() {}

  auto literalStart() const { return literal_start_; }
  auto size() const { return size_; }

  uint8_t *setLiteralPool(void *literalStart, BytecodeArguments &args) {
    literal_start_ = reinterpret_cast<ecma_value_t *>(literalStart);
    size_ = args.literalEnd();

    return reinterpret_cast<uint8_t *>(literal_start_ + size_);
  }

private:
  ecma_value_t *literal_start_;
  uint16_t size_;
};

class Bytecode {
public:
  Bytecode(ecma_value_t function);
  ~Bytecode();

  auto compiledCode() const { return compiled_code_; }
  auto args() const { return args_; }

  auto function() const { return function_; }
  auto flags() const { return flags_; }
  auto literalPool() const { return literal_pool_; }
  auto &stack() { return stack_; }

  void setArguments(cbc_uint16_arguments_t *args);
  void setArguments(cbc_uint8_arguments_t *args);
  void setEncoding();
  void setBytecodeEnd();

  uint8_t next() {
    assert(hasNext());
    return *byte_code_++;
  };
  uint8_t current() { return *byte_code_; };
  bool hasNext() { return byte_code_ < byte_code_end_; };

private:
  void decodeHeader();
  void buildInstructions();

  ecma_object_t *function_;
  ecma_compiled_code_t *compiled_code_;
  uint8_t *byte_code_;
  uint8_t *byte_code_start_;
  uint8_t *byte_code_end_;
  BytecodeFlags flags_;
  BytecodeArguments args_;
  LiteralPool literal_pool_;
  Stack stack_;
};

} // namespace optimizer
#endif // BYTECODE_H
