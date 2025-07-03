#define PY_ARRAY_UNIQUE_SYMBOL Py_Array_API_RFSOC

//#include <boost/shared_ptr.hpp>

#include <core/pybindings.h>
#include <core/G3.h>
#include <core/G3Pipeline.h>
#include <core/G3Writer.h>
#include <RfsocBuilder.h>
#include <RfsocTransmitter.h>
#include <Rfsoc_numpy.h>

#include <pybind11/embed.h>

/* Placeholder file for script that will start a G3Pipeline
 * to catch packets with the minimal working example of
 * the rfsoc-streamer.
 * Modeled on sp3g_software/examples/cppexample.cxx
*/

int main()
{
    // Initializing interpreter and releasing GIL
    // Borrowed from spt3g_software/examples/cppexample.cxx
    // May need changed for our usage, but will leave for now
    py::scoped_interpreter interp;
    py::gil_scoped_release gil;

    // Initializing PyArray_API needed for numpy integration
    //import_array1(0);
    import_array();

    G3Pipeline pipe;
    std::shared_ptr<RfsocBuilder> builder = std::make_shared<RfsocBuilder>();
    RfsocTransmitter* transmitter = new RfsocTransmitter(builder, "192.168.3.58");
    transmitter->Start();

    pipe.Add(builder);
    //pipe.Add(G3ModulePtr(new G3Writer("/home/streamer/data/test.g3")));
    pipe.Add(G3ModulePtr(new G3Writer("/home/ubuntu/test.g3")));

    pipe.Run();

    return 0;
}

