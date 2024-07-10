#pragma once

#include <string>
#include <iostream>

namespace result {
  template <typename T>
  class Result {
  private:
    T result;
    bool err;
    std::string err_msg;
    std::string function_name;
  public:
    void set_err(std::string err_msg, std::string function_name) {
      this->err_msg = err_msg;
      this->err = true;
      this->function_name = function_name;
    }
    void set_result(T result) {
      this->result = result;
      this->err = false;
      this->err_msg = "";
      this->function_name = function_name;
    }
    T unwrap(void) {
      if (!this->err) {
        return this->result;
      }
      std::cerr << "Error in Result from " + this->function_name + ":\n" + err_msg << std::endl;
      exit(1);
    }

    bool is_err(void) {
      return this->err;
    }

    bool is_err(std::string error) {
      if (!this->err) {
        return false;
      }
      else if (this->err_msg == error) {
        return true;
      }
      return false;
    }
  };

  template <typename T>
  Result<T> error(std::string err_msg, std::string function_name) {
    Result<T> res;
    res.set_err(err_msg, function_name);
    return res;
  }
  
  template<typename T>
  Result<T> success(T result) {
    Result<T> res;
    res.set_result(result);
    return res;
  }

  void panic(std::string err_msg, std::string function_name);
}