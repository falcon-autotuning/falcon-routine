#include <gtest/gtest.h>
#include "falcon-routine/math.hpp"
#include <vector>
#include <cmath>
#include <random>

using namespace falcon::routine;

TEST(MathTest, SigmoidEvaluation) {
    double val = sigmoid(0.0, 1.0, 0.0, 1.0, 0.0);
    EXPECT_NEAR(val, 0.5, 1e-6);
}

TEST(MathTest, PiecewiseLinearEvaluation) {
    // x0=0, x1=10, m1=0, m2=1, y0=0
    EXPECT_NEAR(piecewise_linear(-5, 0, 10, 0, 1, 0), 0, 1e-6);
    EXPECT_NEAR(piecewise_linear(5, 0, 10, 0, 1, 0), 5, 1e-6);
    EXPECT_NEAR(piecewise_linear(15, 0, 10, 0, 1, 0), 10, 1e-6);
}

TEST(MathTest, CurveFit1DSimple) {
    // Linear model: y = m*x + b
    auto model = [](std::vector<double> x, std::vector<double> p) {
        std::vector<double> y(x.size());
        for (size_t i = 0; i < x.size(); ++i) {
            y[i] = p[0] * x[i] + p[1];
        }
        return y;
    };

    std::vector<double> x = {0, 1, 2, 3, 4, 5};
    std::vector<double> y = {1, 3, 5, 7, 9, 11}; // m=2, b=1

    fitting_parameters params;
    params.initial_guess = {1.0, 0.0};

    auto res = curvefit1D(model, x, y, params);

    ASSERT_TRUE(res.success);
    EXPECT_NEAR(res.coefficients[0], 2.0, 1e-3);
    EXPECT_NEAR(res.coefficients[1], 1.0, 1e-3);
    EXPECT_GT(res.r_squared, 0.99);
}

TEST(MathTest, CurveFitSigmoid) {
    auto model = [](std::vector<double> x, std::vector<double> p) {
        std::vector<double> y(x.size());
        for (size_t i = 0; i < x.size(); ++i) {
            y[i] = sigmoid(x[i], p[0], p[1], p[2], p[3]);
        }
        return y;
    };

    std::vector<double> x;
    std::vector<double> y;
    double A=1.0, x0=5.0, k=2.0, b=0.1;
    for (double val = 0; val <= 10; val += 0.5) {
        x.push_back(val);
        y.push_back(sigmoid(val, A, x0, k, b));
    }

    fitting_parameters params;
    params.initial_guess = {0.8, 4.5, 1.5, 0.0};

    auto res = curvefit1D(model, x, y, params);

    ASSERT_TRUE(res.success);
    EXPECT_NEAR(res.coefficients[0], A, 1e-2);
    EXPECT_NEAR(res.coefficients[1], x0, 1e-2);
    EXPECT_NEAR(res.coefficients[2], k, 1e-2);
    EXPECT_NEAR(res.coefficients[3], b, 1e-2);
}
