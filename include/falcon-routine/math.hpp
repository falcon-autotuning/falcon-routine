#pragma once
#include "falcon-routine/export.h"
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace falcon {
namespace routine {

// The first vector is the domain (x values) and the second vector is the
// parameters of the model
using func1D = std::function<std::vector<double>(std::vector<double>,
                                                 std::vector<double>)>;
using func2D = std::function<std::vector<std::vector<double>>(
    std::vector<std::vector<double>>, std::vector<std::vector<double>>,
    std::vector<std::vector<double>>)>;

struct FALCON_ROUTINE_API fitting_parameters {
  std::optional<std::vector<double>> initial_guess;
  std::optional<std::vector<std::pair<double, double>>> bounds;
  std::optional<std::string> method;
};

struct FALCON_ROUTINE_API curvefit_result {
  std::vector<double> coefficients;
  std::string error_message;
  bool success;
  double r_squared;
};

enum class AnalysisType {
  TURN_ON,
  PINCH_OFF,
  CHANNEL_ACCUMULATION_2D
};

struct FALCON_ROUTINE_API AnalysisResult {
  AnalysisType type;
  curvefit_result fit;
  std::map<std::string, double> extracted_parameters;
};

/**
 * @brief Performs a curve fit on the given data.
 */
FALCON_ROUTINE_API curvefit_result curvefit1D(func1D model,
                                             const std::vector<double>& x,
                                             const std::vector<double>& y,
                                             fitting_parameters params = {});

/**
 * @brief Performs a 2D curve fit on the given data.
 */
FALCON_ROUTINE_API curvefit_result curvefit2D(func2D model,
                                             const std::vector<std::vector<double>>& x,
                                             const std::vector<std::vector<double>>& y,
                                             const std::vector<std::vector<double>>& z,
                                             fitting_parameters params = {});

// Predefined models
FALCON_ROUTINE_API double sigmoid(double x, double A, double x0, double k, double b);

FALCON_ROUTINE_API double piecewise_linear(double x, double x0, double x1,
                                          double m1, double m2, double y0);

FALCON_ROUTINE_API double channel_accumulation_2d(double x, double y,
                                                 double cx, double cy, double cm, double cr,
                                                 double m1, double m2, double m3,
                                                 double bx, double dx, double dy,
                                                 double dm, double dr);

} // namespace routine
} // namespace falcon
