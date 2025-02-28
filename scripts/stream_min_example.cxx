#define PY_ARRAY_UNIQUE_SYMBOL Py_Array_API_SO3G

//#include <boost/shared_ptr.hpp>

#include <core/pybindings.h>
#include <core/G3.h>
#include <core/G3Pipeline.h>
#include <core/G3Writer.h>
#include <RfsocBuilder.h>
#include <RfsocTransmitter.h>
#include <so3g_numpy.h>

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
    G3PythonInterpreter interp(true);

    // Initializing PyArray_API needed for numpy integration
    import_array1(0);

    G3Pipeline pipe;
    std::shared_ptr<RfsocBuilder> builder = std::make_shared<RfsocBuilder>();
    RfsocTransmitter* transmitter = new RfsocTransmitter(builder, "192.168.3.58");
    transmitter->Start();

    pipe.Add(builder);
    pipe.Add(G3ModulePtr(new G3Writer("/home/streamer/data/test.g3")));

    pipe.Run();

    return 0;
}

