#include <RfsocBuilder.h>
#include <RfsocTransmitter.h>
#include <Rfsoc_numpy.h>

static void* _rfsoc_import_array() {
    import_array();
    return NULL;
}

namespace py = pybind11;

PYBIND11_MODULE(ccatrfsoccore, m) {
    py::module_::import("spt3g.core");

    py::class_<RfsocTransmitter, std::shared_ptr<RfsocTransmitter>>(m, "RfsocTransmitter")
      .def(py::init<G3EventBuilderPtr, std::string>()) // Constructor binding
      .def("Start", &RfsocTransmitter::Start)      // Member function binding
      .def("GetConnectIP", &RfsocTransmitter::GetConnectIP);         // Another member function

    py::class_<RfsocBuilder, G3EventBuilder, std::shared_ptr<RfsocBuilder>>(m, "RfsocBuilder")
      .def(py::init<>()) // Constructor binding
      .def("GetAggDuration", &RfsocBuilder::GetAggDuration)
      .def("SetAggDuration", &RfsocBuilder::SetAggDuration)
      .def("GetDebug", &RfsocBuilder::GetDebug)
      .def("SetDebug", &RfsocBuilder::SetDebug)
      .def("GetEncode", &RfsocBuilder::GetEncode)
      .def("SetEncode", &RfsocBuilder::SetEncode)
      .def("GetDataEncodeAlgo", &RfsocBuilder::GetDataEncodeAlgo)
      .def("SetDataEncodeAlgo", &RfsocBuilder::SetDataEncodeAlgo)
      .def("GetTimeEncodeAlgo", &RfsocBuilder::GetTimeEncodeAlgo)
      .def("SetTimeEncodeAlgo", &RfsocBuilder::SetTimeEncodeAlgo)
      .def("GetEnableCompression", &RfsocBuilder::SetEnableCompression)
      .def("SetEnableCompression", &RfsocBuilder::SetEnableCompression)
      .def("GetBz2WorkFactor", &RfsocBuilder::GetBz2WorkFactor)
      .def("SetBz2WorkFactor", &RfsocBuilder::SetBz2WorkFactor)
      .def("GetFlacLevel", &RfsocBuilder::GetFlacLevel)
      .def("SetFlacLevel", &RfsocBuilder::SetFlacLevel)
      .def("GetDroppedFrames", &RfsocBuilder::GetDroppedFrames);
}
