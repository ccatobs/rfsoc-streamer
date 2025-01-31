#include <RfsocBuilder.h>
#include <RfsocTransmitter.h>
#include <so3g_numpy.h>

namespace bp = boost::python;

static void* _sosmurf_import_array() {
    import_array();
    return NULL;
}

BOOST_PYTHON_MODULE(ccatrfsoccore){
    bp::import("spt3g.core");

    PyEval_InitThreads();
    _sosmurf_import_array();

    RfsocTransmitter::setup_python();
    RfsocBuilder::setup_python();

    printf("Loaded rfsoccore\n");
}
