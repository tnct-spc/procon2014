#ifndef PCS_CONFIG_HPP
#define PCS_CONFIG_HPP

#include <cstdlib>

#ifdef _MSC_VER
std::string const PCS_RESPREF = std::getenv("PCS_RESPREF") ? std::string(std::getenv("PCS_RESPREF")) :"C:\\pcserver";
std::string const PCS_RESDIR = std::getenv("PCS_RESDIR") ? std::string(std::getenv("PCS_RESDIR")) :PCS_RESPREF + "\\res";
std::string const PCS_PROBDIR = std::getenv("PCS_PROBDIR") ? std::string(std::getenv("PCS_PROBDIR")) :PCS_RESPREF + "\\problem_set";
#else
std::string const PCS_RESPREF = std::getenv("PCS_RESPREF") ? std::string(std::getenv("PCS_RESPREF")) : "/usr/share/pcserver";
std::string const PCS_RESDIR = std::getenv("PCS_RESDIR") ? std::string(std::getenv("PCS_RESDIR")) : PCS_RESPREF + "/res";
std::string const PCS_PROBDIR = std::getenv("PCS_PROBDIR") ? std::string(std::getenv("PCS_PROBDIR")) : PCS_RESPREF + "/problem_set";
#endif

#endif

