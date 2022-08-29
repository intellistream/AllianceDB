//https://github.com/jwbuurlage/flags

#include <algorithm>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace Flags {

using namespace std::string_literals;

namespace Detail {

std::vector<std::string> split(std::string s, std::string delim) {
  std::vector<std::string> result;
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delim)) != std::string::npos) {
    token = s.substr(0, pos);
    result.push_back(token);
    s.erase(0, pos + delim.length());
  }
  result.push_back(s);
  return result;
}

enum class flag_type {
  optional,
  required,
  flag,
};

} // namespace Detail

class Flags {
 public:
  Flags(int argc, char **argv) : argc_(argc), argv_(argv) {}

  void info(std::string program_name, std::string description) {
    program_name_ = program_name;
    description_ = description;
  }

  bool passed(std::string flag) {
    flags_.emplace_back(flag, Detail::flag_type::flag, ""s);
    return passed_(flag);
  }

  std::string arg(std::string flag) {
    flags_.emplace_back(flag, Detail::flag_type::required, ""s);
    return arg_(flag);
  }

  std::vector<std::string> args(std::string flag) {
    flags_.emplace_back(flag, Detail::flag_type::optional, "[]");

    auto pos = std::find(argv_, argv_ + argc_, flag);
    std::vector<std::string> result;
    if (pos == argv_ + argc_ || pos + 1 == argv_ + argc_) {
      return result;
    }
    pos++;
    while (pos != argv_ + argc_ && *pos[0] != '-') {
      result.push_back(std::string(*pos));
      pos++;
    }

    return result;
  }

  template<typename T>
  T arg_as(std::string flag) {
    flags_.emplace_back(flag, Detail::flag_type::required, "");

    auto value = std::stringstream(arg_(flag));
    T x = {};
    value >> x;
    return x;
  }

  template<typename T>
  std::vector<T> args_as(std::string flag) {
    flags_.emplace_back(flag, Detail::flag_type::optional, "[]");

    auto parts = Detail::split(arg_(flag), ",");
    std::vector<T> xs;
    for (auto part: parts) {
      auto value = std::stringstream(part);
      T x = {};
      value >> x;
      xs.push_back(x);
    }

    return xs;
  }

  template<typename T>
  T arg_as_or(std::string flag, T alt) {
    flags_.emplace_back(flag, Detail::flag_type::optional,
                        std::to_string(alt));

    if (!passed_(flag)) {
      return alt;
    }
    auto value = std::stringstream(arg_(flag));
    T x = {};
    value >> x;
    return x;
  }

  std::string arg_or(std::string flag, std::string alt) {
    flags_.emplace_back(flag, Detail::flag_type::optional, alt);

    if (!passed_(flag)) {
      return alt;
    }
    return arg_(flag);
  }

  bool required_arguments(const std::vector<std::string> &args) {
    for (auto &arg: args) {
      if (!passed_(arg)) {
        return false;
      }
    }
    return true;
  }

  bool sane() {
    for (auto [flag, type, alt]: flags_) {
      (void) alt;
      if (type == Detail::flag_type::required && !passed_(flag)) {
        return false;
      }
    }
    return true;
  }

  std::string usage() {
    auto flag_size = [](auto &xs) { return std::get<0>(xs).size(); };

    auto max_flag_size = flag_size(*std::max_element(
        flags_.begin(), flags_.end(), [&](auto &lhs, auto &rhs) {
          return flag_size(lhs) < flag_size(rhs);
        }));

    auto output = std::stringstream("");

    if (program_name_.empty()) {
      program_name_ = argv_[0];
    }
    output << program_name_ << "\n";
    if (!description_.empty()) {
      output << description_ << "\n";
    }
    output << "\n";

    output << "USAGE: " << program_name_ << " [OPTIONS]\n\n";

    output << "OPTIONS:\n";
    for (auto [flag, type, alt]: flags_) {
      output << "    " << flag;

      auto padding = max_flag_size - flag.size();
      output << std::string(padding, ' ');
      if (type == Detail::flag_type::optional) {
        output << "    DEFAULT: " << alt;
      }
      if (type == Detail::flag_type::flag) {
        output << "    FLAG";
      }

      output << "\n";
    }
    return output.str();
  }

 private:
  bool passed_(std::string flag) {
    return std::find(argv_, argv_ + argc_, flag) != (argv_ + argc_);
  }

  std::string arg_(std::string flag) {
    auto pos = std::find(argv_, argv_ + argc_, flag);
    if (pos == argv_ + argc_ || pos + 1 == argv_ + argc_) {
      return "";
    }
    pos++;
    return std::string(*pos);
  }

  int argc_;
  char **argv_;
  std::vector<std::tuple<std::string, Detail::flag_type, std::string>> flags_;
  std::string program_name_;
  std::string description_;
};

} // namespace flags
