/**
 * @Author: Tristan Croll <tic20>
 * @Date:   30-Sep-2020
 * @Email:  tic20@cam.ac.uk
 * @Last modified by:   tic20
 * @Last modified time: 30-Sep-2020
 * @License: Free for non-commercial use (see license.pdf)
 * @Copyright: 2016-2019 Tristan Croll
 */

#include <pybind11/pybind11.h>
#include "simple_async.h"

namespace py=pybind11;

PYBIND11_MODULE(simple_async, m) {
    py::class_<Foo>(m, "AsyncHelloWorld")
        .def(py::init<>())
        .def("do_something_slow", &Foo::do_something_slow)
        .def("do_something_slow_asynchronously", &Foo::do_something_slow_asynchronously)
        .def("thread_done", &Foo::thread_done)
        .def("get_asynchronous_result", &Foo::get_asynchronous_result)
        ;
}
