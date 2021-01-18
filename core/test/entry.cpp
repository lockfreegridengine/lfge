#define CAF_TEST_NO_MAIN

#include "caf/test/unit_test_impl.hpp"
#include "core/core_atoms.hpp"

int main(int argc, char** argv) {
  using namespace caf;
  init_global_meta_objects< caf::id_block::lfge_core >();
  core::init_global_meta_objects();
  return test::main(argc, argv);
}
