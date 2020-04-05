esp-idf component for [eshetcpp](https://github.com/tomjnixon/eshetcpp).

## Using it

Add this repository as a submodule in `components`:

    git submodule add git@github.com:tomjnixon/eshet-esp-idf.git components/eshet

Add `eshet` to your `main` component's `PRIV_REQUIRES`; in `main/CMakeLists.txt` this looks like:

    idf_component_register(SRCS "main.cpp" PRIV_REQUIRES eshet)

Enable C++ exceptions by adding this to `sdkconfig.defaults`:

    CONFIG_COMPILER_CXX_EXCEPTIONS=y

(is there a way to require that this is set from a component?)
