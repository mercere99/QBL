#pragma once

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/String.hpp"

static inline emp::String LineToRawText(emp::String line) {
  emp::String out_line;

  // Everything between backslash \& and ; or \< to > ignore.
  char scan_to = '\0';
  bool start_scan = false;
  emp::String scan_word;

  // If we have a negative value, we need to wait for another char to know the symbol
  char partial = '\0';
  for (char c : line) {
    if (partial) {
      int val1 = static_cast<int>(partial);
      int val2 = static_cast<int>(c);
      switch (val1) {
      case -50:
        switch (val2) {
        case -87: out_line += "O"; break;
        case -104: out_line += "T"; break;
        default:
          emp::notify::Error("Unknown char combo: ", val1, ",", val2, "\nline: ", line);          
        }
        break;
      default:
        emp::notify::Error("Unknown char combo: ", val1, ",", val2, "\nline: ", line);          
      }
      partial = '\0';
      continue;
    }
    if (c < 0) {
      partial = c;
      continue;
    }

    if (scan_to) {  // Do we need to literally translate?
      if (scan_to == c) {
        if (c == ';') {
          if (scan_word == "Theta") out_line += "T";
          else if (scan_word == "Omega") out_line += "O";
        }
        scan_to = '\0';
        scan_word = "";
      } else {
        scan_word += c;
      }
      continue;
    }

    if (start_scan) {
      switch (c) {
      case '&': scan_to = ';'; break;
      case '<': scan_to = '>'; break;
      case '\\': out_line += c; break;
      case 'n': out_line += "\\\\ "; break;
      default:
        std::cerr << "Error: Unknown escape character '" << c << "'.\n" << std::endl;
        exit(1);
      }
      start_scan = false;
      continue;
    }

    switch (c) {
      case '\\': start_scan = true; break;
      case '`':  break;
      default:
        out_line += c;
        break;
    }
  }

  return out_line;
}

// Convert a single line of text to D2L format.
static inline emp::String LineToD2L(emp::String line) {
  emp::notify::TestError(line.Has('\n'), "Newline found inside of line: ", line);
  emp::String out_line;

  bool in_codeblock = line.HasPrefix("    ");
  bool in_code = in_codeblock;

  // Everything between backslash \& and ; or \< to > make literal
  char scan_to = '\0';
  bool start_scan = false;

  if (in_codeblock) {
    line.PopFixed(4);
    out_line += "&nbsp;&nbsp;<code>";
  }

  for (char c : line) {
    if (scan_to) {
      out_line += c;
      if (scan_to == c) scan_to = '\0';
      continue;
    }

    if (start_scan) {
      switch (c) {
      case '&': out_line += c; scan_to = ';'; break;
      case '<': out_line += c; scan_to = '>'; break;
      case '\\': out_line += c; break;
      case '\n': out_line += "<br>"; break;
      default:
        std::cerr << "Error: Unknown escape character '" << c << "'.\n" << std::endl;
        exit(1);
      }
      start_scan = false;
      continue;
    }

    switch (c) {
      case '\"': out_line += "&quot;"; break;
      case ' ': out_line += in_code ? "&nbsp;" : " ";  break;
      case ',': out_line += "&#44;";   break;
      case '<': out_line += in_code ? "&lt;" : "<"; break;  // Outside code might be HTML
      case '>': out_line += in_code ? "&gt;" : ">"; break;  // Outside code might be HTML
      case '&': out_line += in_code ? "&amp;" : "&"; break;  // Outside code might be HTML
      case '\\': start_scan = true; break;

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

  // Everything between backslash \& and ; or \< to > make literal
  char scan_to = '\0';
  bool start_scan = false;
  emp::String scan_word;

  // If we have a negative value, we need to wait for another char to know the symbol
  char partial = '\0';

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
    if (partial) {
      int val1 = static_cast<int>(partial);
      int val2 = static_cast<int>(c);
      switch (val1) {
      case -50:
        switch (val2) {
        case -87: out_line += "$\\Omega$"; break;
        case -104: out_line += "$\\Theta$"; break;
        default:
          emp::notify::Error("Unknown char combo: ", val1, ",", val2, "\nline: ", line);          
        }
        break;
      default:
        emp::notify::Error("Unknown char combo: ", val1, ",", val2, "\nline: ", line);          
      }
      partial = '\0';
      continue;
    }
    if (c < 0) {
      partial = c;
      continue;
    }

    if (scan_to) {  // Do we need to literally translate?
      if (scan_to == c) {
        if (c == ';') {
          if (scan_word == "Theta") out_line += "$\\Theta$";
          else if (scan_word == "Omega") out_line += "$\\Omega$";
        }
        else if (c == '>') {
          if (scan_word == "b") out_line += "\\textbf{";
          else if (scan_word == "/b") out_line += "}";
          else if (scan_word == "i") out_line += "\\textit{";
          else if (scan_word == "/i") out_line += "}";
          else if (scan_word == "sup") out_line += "\\textsuperscript{";
          else if (scan_word == "/sup") out_line += "}";
          else if (scan_word == "sub") out_line += "\\textsubscript{";
          else if (scan_word == "/sub") out_line += "}";
        }
        scan_to = '\0';
        scan_word = "";
      } else {
        scan_word += c;
      }
      continue;
    }

    if (start_scan) {
      switch (c) {
      case '&': scan_to = ';'; break;
      case '<': scan_to = '>'; break;
      case '\\': out_line += c; break;
      case 'n': out_line += "\\\\ "; break;
      default:
        std::cerr << "Error: Unknown escape character '" << c << "'.\n" << std::endl;
        exit(1);
      }
      start_scan = false;
      continue;
    }

    switch (c) {
      case '{': out_line += "\\{";  break;
      case '}': out_line += "\\}";  break;
      case '%': out_line += "\\%";  break;
      case '$': out_line += "\\$";  break;
      case '~': out_line += "\\~";  break;
      case '#': out_line += "\\#";  break;
      case '_': out_line += "\\_";  break;
      case '\\': start_scan = true; break;

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

  // Everything between backslash \& and ; or \< to > make literal
  char scan_to = '\0';
  bool start_scan = false;

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
    if (scan_to) {  // Do we need to literally translate?
      out_line += c;
      if (scan_to == c) scan_to = '\0';
      continue;
    }

    if (start_scan) {
      switch (c) {
      case '&': out_line += c; scan_to = ';'; break;
      case '<': out_line += c; scan_to = '>'; break;
      case '\\': out_line += c; break;
      case 'n': out_line += "<br>"; break;
      default:
        std::cerr << "Error: Unknown escape character '" << c << "'.\n" << std::endl;
        exit(1);
      }
      start_scan = false;
      continue;
    }

    switch (c) {
      case '&': out_line += "&amp;";  break;
      case '<': out_line += "&lt;"; break;
      case '>': out_line += "&gt;"; break;
      case '\'': out_line += "&apos;"; break;
      case '"': out_line += "&quot;"; break;
      case '\\': start_scan = true; break;

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

// Convert a whole text block to Raw Text format.
static inline emp::String TextToRawText(const emp::String & text) {
  emp::vector<emp::String> lines = text.Slice("\n");
  for (auto & line : lines) line = LineToRawText(line);
  return emp::Join(lines, "\n");
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
