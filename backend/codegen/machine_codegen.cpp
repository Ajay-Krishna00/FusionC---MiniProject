// Executes the TAC program produced by the middle-end.
#include "machine_codegen.h"

#include <unordered_map>

namespace fusionc::backend::codegen
{

  ExecutionResult execute(const middleend::ir::Program &program)
  {
    std::unordered_map<std::string, int> slots;
    std::unordered_map<std::string, size_t> labels;
    ExecutionResult result;
    size_t pc = 0;

    // First pass: collect labels
    for (size_t i = 0; i < program.size(); ++i)
    {
      if (program[i].op == "label")
      {
        labels[program[i].dst] = i;
      }
    }

    auto read = [&](const std::string &name) -> int
    {
      const auto it = slots.find(name);
      if (it != slots.end())
      {
        return it->second;
      }
      return std::stoi(name); // fall back to literal
    };

    while (pc < program.size())
    {
      const auto &ins = program[pc];
      if (ins.op == "const")
      {
        slots[ins.dst] = std::stoi(ins.arg1);
      }
      else if (ins.op == "copy")
      {
        slots[ins.dst] = read(ins.arg1);
      }
      else if (ins.op == "add")
      {
        slots[ins.dst] = read(ins.arg1) + read(ins.arg2);
      }
      else if (ins.op == "sub")
      {
        slots[ins.dst] = read(ins.arg1) - read(ins.arg2);
      }
      else if (ins.op == "mul")
      {
        slots[ins.dst] = read(ins.arg1) * read(ins.arg2);
      }
      else if (ins.op == "div")
      {
        slots[ins.dst] = read(ins.arg1) / read(ins.arg2);
      }
      else if (ins.op == "lt")
      {
        slots[ins.dst] = read(ins.arg1) < read(ins.arg2) ? 1 : 0;
      }
      else if (ins.op == "gt")
      {
        slots[ins.dst] = read(ins.arg1) > read(ins.arg2) ? 1 : 0;
      }
      else if (ins.op == "eq")
      {
        slots[ins.dst] = read(ins.arg1) == read(ins.arg2) ? 1 : 0;
      }
      else if (ins.op == "ret")
      {
        result.ok = true;
        result.exitCode = read(ins.dst);
        result.message = "Program executed";
        return result;
      }
      else if (ins.op == "label")
      {
        // do nothing
      }
      else if (ins.op == "jmp")
      {
        pc = labels[ins.dst];
        continue;
      }
      else if (ins.op == "jz")
      {
        if (read(ins.dst) == 0)
        {
          pc = labels[ins.arg1];
          continue;
        }
      }
      ++pc;
    }

    result.ok = true;
    result.message = "Program executed without explicit return; exit code 0";
    result.exitCode = 0;
    return result;
  }

} // namespace fusionc::backend::codegen
