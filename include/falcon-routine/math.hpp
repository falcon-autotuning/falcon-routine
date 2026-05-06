#pragma once
#include "hub.hpp"
#include <functional>
#include <map>
using namespace falcon::routine;
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
/*
 * @brief Performs a curve fit on the given data and returns the coefficients of
 * the fit.
 * @param model the model to fit to the data. The function should take in a
 * vector of doubles representing the x values and return a vector of doubles
 * representing the y values.
 * @param x the x values of the data to fit to
 * @param y the y values of the data to fit to
 * @param params the parameters for the fitting process, including an optional
 * initial guess and bounds for the coefficients
 * @return a curvefit_result struct containing the coefficients of the fit, an
 * error and a success flag
 */
curvefit_result curvefit1D(func1D model, std::vector<double> x,
                           std::vector<double> y,
                           fitting_parameters params = {});
/*
 * @brief Performs a curve fit on the given data and returns the coefficients of
 * the fit.
 * @param model the model to fit to the data. The function should take in a
 * vector of doubles representing the x values and return a vector of doubles
 * representing the y values.
 * @param x the x values of the data to fit to
 * @param y the y values of the data to fit to
 * @param params the parameters for the fitting process, including an optional
 * initial guess and bounds for the coefficients
 * @return a curvefit_result struct containing the coefficients of the fit, an
 * error and a success flag
 */
curvefit_result curvefit2D(func2D model, std::vector<std::vector<double>> x,
                           std::vector<std::vector<double>> y,
                           std::vector<std::vector<double>> z,
                           fitting_parameters params = {});
