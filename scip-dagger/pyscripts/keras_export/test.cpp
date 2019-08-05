#include <vector>
#include <thread>
#include <mutex>
#include <fdeep/fdeep.hpp>

#if 1
int main()
{
  float cdata[] = {0.211, -0.18448, -1, 1, -1, 0.19814, -0.0011, -1, -1, -1.0 ,-1, -1 , -1, -1, 1, 1, -1, -0.982, -1, -1, -1, -1, -1, 1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    std::vector<float> test;
  for (int i = 0; i <36; i++)
    test.push_back(cdata[i]);

  const auto model = fdeep::load_model("/Users/subrahma/proj/maverick-setup/co-training/scip-dagger/policy//Users/subrahma/proj/maverick-setup/co-training//data//psulu_small_a//lpfiles/train/nn_psulu_0.25_cotrain_search_lp_0/searchPolicy.0_keras.json");
  const auto result = model.predict({fdeep::tensor3(fdeep::shape_hwc(1, 1, 36), {0.211, -0.18448, -1, 1, -1, 0.19814, -0.0011, -1, -1, -1.0 ,-1, -1 , -1, -1, 1, 1, -1, -0.982, -1, -1, -1, -1, -1, 1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1} )});
  std::cout << fdeep::show_tensor3s(result) << std::endl;
}
#else
int main()
{
const auto model = fdeep::load_model("fdeep_model.json");
const auto result = model.predict(
{fdeep::tensor3(fdeep::shape_hwc(1, 1, 4), {1, 2, 3, 4})});
std::cout << fdeep::show_tensor3s(result) << std::endl;
}
#endif
