#pragma once

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/String.hpp"

// Convert a single line of text to D2L format.
static inline emp::String LineToD2L(emp::String line) {
  emp::notify::TestError(line.Has('\n'), "Newline found inside of line: ", line);
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

static inline emp::String LineToLatex(emp::String line) {
  emp::notify::TestError(line.Has('\n'), "Newline found inside of line: ", line);
  emp::String out_line;

  bool in_codeblock = line.HasPrefix("    ");
  bool in_code = in_codeblock;

  if (in_codeblock) {
    line.PopFixed(4);
    out_line += "\\texttt{";

    size_t ws_count = 0;
    while (ws_count < line.size() && line[ws_count] == ' ') ws_count++;
    if (ws_count) {
      out_line += emp::MakeString("\\hspace*{", ws_count, "em}");
      line.PopFixed(ws_count);
    }
  }

  for (char c : line) {
    switch (c) {
      case '{': out_line += "\\{";  break;
      case '}': out_line += "\\}";  break;
      case '%': out_line += "\\%";  break;
      case '$': out_line += "\\$";  break;
      case '~': out_line += "\\~";  break;
      case '#': out_line += "\\#";  break;
      case '_': out_line += "\\_";  break;

      // Replace ` with \texttt{ or }
      case '`':
        if (in_codeblock) {
          out_line += '`';
        } else {
          if (in_code) out_line += "}";
          else out_line += "\\texttt{";
          in_code = !in_code;
        }
        break;

      default:
        out_line += c;
        break;
    }
  }

  // If we are in code at the end of the entry, close it off.
  if (in_code) out_line += "}";

  return out_line;
}

static inline emp::String LineToHTML(emp::String line) {
  emp::notify::TestError(line.Has('\n'), "Newline found inside of line: ", line);
  emp::String out_line;

  bool in_codeblock = line.HasPrefix("    ");
  bool in_code = in_codeblock;

  if (in_codeblock) {
    line.PopFixed(4);
    out_line += "&nbsp;&nbsp;<code>";

    size_t ws_count = 0;
    while (ws_count < line.size() && line[ws_count] == ' ') ws_count++;
    if (ws_count) {
      out_line += emp::MakeRepeat("&nbsp;", ws_count);
      line.PopFixed(ws_count);
    }
  }

  for (char c : line) {
    switch (c) {
      case '&': out_line += "&amp;";  break;
      case '<': out_line += "&lt;"; break;
      case '>': out_line += "&gt;"; break;
      case '\'': out_line += "&apos;"; break;
      case '"': out_line += "&quot;"; break;

      // Replace ` with \texttt{ or }
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

// Convert a whole text block to Latex format.
static inline emp::String TextToLatex(const emp::String & text) {
  emp::vector<emp::String> lines = text.Slice("\n");
  for (auto & line : lines) line = LineToLatex(line);
  return emp::Join(lines, "\\\\\n");
}

// Convert a whole text block to HTML format.
static inline emp::String TextToHTML(const emp::String & text) {
  emp::vector<emp::String> lines = text.Slice("\n");
  for (auto & line : lines) line = LineToHTML(line);
  return emp::Join(lines, "<br>\n");
}
