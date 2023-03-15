#pragma once

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/String.hpp"

// Convert a single line of text to D2L format.
static inline emp::String LineToD2L(emp::String line) {
  emp::String out_line;

  bool in_codeblock = line.HasPrefix("    ");
  bool in_code = in_codeblock;

  if (in_codeblock) {
    line.PopFixed(4);
    out_line += "&nbsp;&nbsp;<code>";
  }

  for (char c : line) {
    switch (c) {
      case '\"': out_line += "&quot;"; break;
      case ' ': out_line += in_code ? "&nbsp;" : " ";  break;
      case ',': out_line += "&#44;";   break;
      case '<': out_line += in_code ? "&lt;" : "<"; break;  // Outside code might be HTML
      case '>': out_line += in_code ? "&gt;" : ">"; break;  // Outside code might be HTML
      case '&': out_line += in_code ? "&amp;" : "&"; break;  // Outside code might be HTML

      // Replace ` with <code> or </code>
      case '`':
        if (in_codeblock) {
          out_line += '`';
        } else {
          if (in_code) out_line += "</code>";
          else out_line += "<code>";
          in_code = !in_code;
        }
        break;

      default:
        out_line += c;
        break;
    }
  }

  // If we are in code at the end of the entry, close it off.
  if (in_code) out_line += "</code>";

  return out_line;
}

// Convert a whole text block to D2L format.
static inline emp::String TextToD2L(const emp::String & text) {
  emp::vector<emp::String> lines = text.Slice("\n");
  for (auto & line : lines) line = LineToD2L(line);
  return emp::Join(lines, "<br>");
}
