#include <core/pybindings.h>
#include <core/G3.h>
#include <core/G3Pipeline.h>
#include <core/G3Writer.h>
#include <RfsocBuilder.h>
#include <RfsocTransmitter.h>

// Need to include spt3g_software? So3g?

/* Placeholder file for script that will start a G3Pipeline
 * to catch packets with the minimal working example of
 * the rfsoc-streamer.
 * Modeled on sp3g_software/examples/cppexample.cxx
*/

int main{

    // Initializing interpreter and releasing GIL
    // Borrowed from spt3g_software/examples/cppexample.cxx
    // May need changed for our usage, but will leave for now
    G3PythonInterpreter interp(false);

    G3Pipeline pipe;
    RfsocBuilder builder;
    RfsocTransmitter transmitter(builder);
    transmitter.Start();

    pipe.Add(builder);
    pipe.Add(G3ModulePtr(new G3Writer("/data/test.g3")));

    pipe.Run();

    return 0;
}