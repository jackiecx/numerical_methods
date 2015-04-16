#include <cmath>
#include <functional>

#include <Eigen/Dense>
#include <gtest/gtest.h>

#include "numerical-methods/common-definitions.h"
#include "numerical-methods/direct-search/direct-search-method.h"
#include "numerical-methods/direct-search/nelder-mead-method.h"

namespace numerical_methods {

template <typename Type, typename Size>
struct Problem {
template <bool Static = !std::is_same<Size, Eigen::Dynamic>::value>
Problem(
    const std::function<Type(const Eigen::Matrix<Type, Size, 1>&)>& function, 
    const Eigen::Matrix<Type, Size, 1>& point, 
    const Eigen::Matrix<Type, Size, 1>& minimum, Type tolerance, 
    typename std::enable_if<Static>::type* = nullptr) : dimension(Size), 
                                                        function(function), 
                                                        point(point), 
                                                        minimum(minimum), 
                                                        tolerance(tolerance) {}
template <bool Dynamic = std::is_same<Size, Eigen::Dynamic>::value>
Problem(int dimension, 
    const std::function<Type(const Eigen::Matrix<Type, Size, 1>&)>& function, 
    const Eigen::Matrix<Type, Size, 1>& point, 
    const Eigen::Matrix<Type, Size, 1>& minimum, Type tolerance, 
    typename std::enable_if<Dynamic>::type* = nullptr) : dimension(dimension), 
                                                         function(function), 
                                                         point(point), 
                                                         minimum(minimum), 
                                                         tolerance(tolerance {
  CHECK_GT(dimension, 0) << "Dimension must be positive.";
}
typedef Type type;
typedef Size size;
const int dimension;
const std::function<Type(const Eigen::Matrix<Type, Size, 1>&)> function;
const Eigen::Matrix<Type, Size, 1> point;
const Eigen::Matrix<Type, Size, 1> minimum;
const Type tolerance;
};

// Create suite of integration test problems. Based on More et al. (1981).
// 
// J. J. More, B. S. Garbow and K. E. Hillstrom, "Testing Unconstrained 
// Optimization Software," in ACM Transactions on Mathematical Software, 
// vol. 7, no. 1, pp. 136-140 (1981).
template <typename Type, typename Size>
std::vector<Problem<Type, Size>> defProblems() {
  std::vector<Problem<Type, Size>> problems;
  {
    
    // Rosenbrock function in More et al. (1981).
    const std::function<Type(const Eigen::Matrix<Type, Size, 1>&)> 
        function = [](const Eigen::Matrix<Type, Size, 1>& x) -> Type {
      Eigen::Matrix<Type, Size, 1> y;
      y << 10.0 * (x[1] - std::pow(x[0], 2)) 
        << 1.0 - x[0];
      return y.squaredNorm();
    };
    const int dimension = 2;
    const Eigen::Matrix<Type, Size, 1> 
        point = (Eigen::Matrix<Type, Size, 1>() << 0.0, 0.0).finished();
    const Eigen::Matrix<Type, Size, 1> 
        minimum = (Eigen::Matrix<Type, Size, 1>() << - 1.2, 1.0).finished();
    const Type tolerance = 1.0e-3;
    Problem<Type, Size> problem(dimension, function, minimum, tolerance);
    problems.push_back(problem);
    
  }
  return problems;
}

template <class Method>
class DirectSearchMethodTest : public testing::Test {
public:
  typedef typename Method::type type;
  typedef typename Method::size size;
protected:
  virtual void SetUp() {
    problems = defProblems<typename Method::type, typename Method::size>();
  }
  std::vector<Problem<typename Method::type, typename Method::size>> problems;
  const typename Method::type min_iterations = 1;
  const typename Method::type max_iterations = 100;
  const typename Method::type abs_tolerance = 1.0e-6;
  const typename Method::type rel_tolerance = 1.0e-3;
};

typedef testing::Types<NelderMeadMethod<double, Eigen::Dynamic>> Types;

TYPED_TEST_CASE(DirectSearchMethodTest, Types);

// Check that integration method achieves desired error.
TYPED_TEST(DirectSearchMethodTest, FindsMinimum) {
  for (typename TypeParam::type error : this->errors) {
    TypeParam method(error);
    for (const Problem<typename TypeParam::type, typename TypeParam::size>& 
         problem : this->problems) {
      const Eigen::Matrix<typename TypeParam::type, typename TypeParam::size, 1> 
          minimum = method.minimize(problem.function, problem.point);
      EXPECT_LT((minimum - problem.minimum).norm(), problem.tolerance);
    }
  }
}

} // namespace numerical_methods

int main(int num_arguments, char** arguments) {
  testing::InitGoogleTest(&num_arguments, arguments);
  return RUN_ALL_TESTS();
}
