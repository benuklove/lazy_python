#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

#include "lazy.h"

/* We can only use matmul on 3.5+. */
#define LZ_HAS_MATMUL PY_MINOR_VERSION >= 5
#define STR(a) # a

typedef struct{
    PyObject_HEAD
    const char *wr_name;
    void *wr_func;
}callablewrapper;

static PyObject *
callablewrapper_repr(callablewrapper *self)
{
    return PyUnicode_FromFormat(
        "<wrapped-function %s>",
        self->wr_name);
}

static PyMemberDef callablewrapper_members[] = {
    {"__name__", T_STRING, offsetof(callablewrapper, wr_name), READONLY, ""},
    {NULL},
};

/* Binary  wrapper --------------------------------------------------------- */

static PyTypeObject binwrapper_type;

static PyObject *
binwrapper_call(callablewrapper *self, PyObject *args, PyObject *kwargs)
{
    if (kwargs && PyDict_Size(kwargs)) {
        PyErr_SetString(PyExc_TypeError,
                        "callable does not accept keyword arguments");
        return NULL;
    }

    if (PyTuple_GET_SIZE(args) != 2) {
        PyErr_Format(PyExc_TypeError,
                     "callable expects 2 arguments, passed %zd",
                     PyTuple_GET_SIZE(args));
        return NULL;
    }

    return ((PyObject *(*)(PyObject*,PyObject*)) self->wr_func)(
        PyTuple_GET_ITEM(args, 0),
        PyTuple_GET_ITEM(args, 1));
}

PyDoc_STRVAR(callablewrapper_doc, "A wrapper for a c functions.");

static PyTypeObject binwrapper_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "lazy._thunk.binwrapper",                   /* tp_name */
    sizeof(callablewrapper),                    /* tp_basicsize */
    0,                                          /* tp_itemsize */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc) callablewrapper_repr,            /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    (ternaryfunc) binwrapper_call,              /* tp_call */
    (reprfunc) callablewrapper_repr,            /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    callablewrapper_doc,                        /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    callablewrapper_members,                    /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};

#define BINWRAPPER_FROM_FUNC(func, name)                                \
    callablewrapper __binwrapper_ ## name = {                           \
        PyObject_HEAD_INIT(&binwrapper_type)                            \
        STR(name),                                                      \
        func                                                            \
    };                                                                  \
    PyObject *LzBinary_ ## name = (PyObject*) &__binwrapper_ ## name;

/* Unary wrapper ----------------------------------------------------------- */

static PyTypeObject unarywrapper_type;

static PyObject *
unarywrapper_call(callablewrapper *self, PyObject *args, PyObject *kwargs)
{
    if (kwargs && PyDict_Size(kwargs)) {
        PyErr_SetString(PyExc_TypeError,
                        "callable does not accept keyword arguments");
        return NULL;
    }

    if (PyTuple_GET_SIZE(args) != 1) {
        PyErr_Format(PyExc_TypeError,
                     "callable expects 1 argument, passed %zd",
                     PyTuple_GET_SIZE(args));
        return NULL;
    }

    return ((PyObject *(*)(PyObject*)) self->wr_func)(
        PyTuple_GET_ITEM(args, 0));
}

static PyTypeObject unarywrapper_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "lazy._thunk.unarywrapper",                 /* tp_name */
    sizeof(callablewrapper),                    /* tp_basicsize */
    0,                                          /* tp_itemsize */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc) callablewrapper_repr,            /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    (ternaryfunc) unarywrapper_call,            /* tp_call */
    (reprfunc) callablewrapper_repr,            /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    callablewrapper_doc,                        /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    callablewrapper_members,                    /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};

#define UNARYWRAPPER_FROM_FUNC(func, name)                              \
    callablewrapper __unarywrapper_ ## name = {                         \
        PyObject_HEAD_INIT(&unarywrapper_type)                          \
        STR(name),                                                      \
        func                                                            \
    };                                                                  \
    PyObject *LzUnary_ ## name = (PyObject*) &__unarywrapper_ ## name;

/* Ternary wrapper --------------------------------------------------------- */

static PyTypeObject ternarywrapper_type;

static PyObject *
ternarywrapper_call(callablewrapper *self, PyObject *args, PyObject *kwargs)
{
    if (kwargs && PyDict_Size(kwargs)) {
        PyErr_SetString(PyExc_TypeError,
                        "callable does not accept keyword arguments");
        return NULL;
    }

    if (PyTuple_GET_SIZE(args) != 3) {
        PyErr_Format(PyExc_TypeError,
                     "callable expects 3 arguments, passed %zd",
                     PyTuple_GET_SIZE(args));
        return NULL;
    }

    return ((PyObject *(*)(PyObject*, PyObject*, PyObject*)) self->wr_func)(
        PyTuple_GET_ITEM(args, 0),
        PyTuple_GET_ITEM(args, 1),
        PyTuple_GET_ITEM(args, 2));
}

static PyTypeObject ternarywrapper_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "lazy._thunk.ternarywrapper",               /* tp_name */
    sizeof(callablewrapper),                    /* tp_basicsize */
    0,                                          /* tp_itemsize */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc) callablewrapper_repr,            /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    (ternaryfunc) ternarywrapper_call,          /* tp_call */
    (reprfunc) callablewrapper_repr,            /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    callablewrapper_doc,                        /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    callablewrapper_members,                    /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};

#define TERNARYWRAPPER_FROM_FUNC(func, name)                            \
    callablewrapper __ternarywrapper_ ## name = {                       \
        PyObject_HEAD_INIT(&ternarywrapper_type)                        \
        STR(name),                                                      \
        func                                                            \
    };                                                                  \
    PyObject *LzTernary_ ## name = (PyObject*) &__ternarywrapper_ ## name;

/* thunk ------------------------------------------------------------------- */

typedef struct{
    PyObject_HEAD
    PyObject *th_func;
    PyObject *th_args;
    PyObject *th_kwargs;
    PyObject *th_normal;
}thunk;

static PyTypeObject thunk_type;
static PyObject *thunk_fromvalue(PyTypeObject *cls, PyObject *value);

/* strict ------------------------------------------------------------------- */

static PyObject *strict_eval(PyObject*);

/* Strictly evaluate a thunk.
   return: A borrowed reference. */
static PyObject *
_strict_eval_borrowed(PyObject *self)
{
    PyObject *normal_func;
    PyObject *normal_args;
    PyObject *normal_kwargs;
    Py_ssize_t nargs;
    Py_ssize_t n;
    PyObject *arg;
    PyObject *key;
    PyObject *value;

    if (!((thunk*) self)->th_normal) {
        if (!(normal_func = strict_eval(((thunk*) self)->th_func))) {
            return NULL;
        }

        nargs = PyTuple_GET_SIZE(((thunk*) self)->th_args);
        if (!(normal_args = PyTuple_New(nargs))) {
            Py_DECREF(normal_func);
            return NULL;
        }

        for (n = 0;n < nargs;++n) {
            if (!(arg = strict_eval(
                      PyTuple_GET_ITEM(((thunk*) self)->th_args, n)))) {
                Py_DECREF(normal_func);
                Py_DECREF(normal_args);
                return NULL;
            }
            PyTuple_SET_ITEM(normal_args, n, arg);
        }

        if (((thunk*) self)->th_kwargs) {
            if (!(normal_kwargs = PyDict_Copy(((thunk*) self)->th_kwargs))) {
                Py_DECREF(normal_func);
                Py_DECREF(normal_args);
                return NULL;
            }

            n = 0;
            while (PyDict_Next(normal_args, &n, &key, &value)) {
                if ((!(arg = strict_eval(value))) ||
                    PyDict_SetItem(normal_args, key, arg)) {

                    Py_DECREF(normal_func);
                    Py_DECREF(normal_args);
                    Py_DECREF(normal_kwargs);
                    return NULL;
                }
            }
        }
        else {
            normal_kwargs = NULL;
        }

        ((thunk*) self)->th_normal = PyObject_Call(normal_func,
                                                   normal_args,
                                                   normal_kwargs);
        Py_DECREF(normal_func);
        Py_DECREF(normal_args);
        Py_XDECREF(normal_kwargs);
        if (!((thunk*) self)->th_normal) {
            return NULL;
        }
        /* Remove the references to the function and args to not persist
           these references. */
        Py_CLEAR(((thunk*) self)->th_func);
        Py_CLEAR(((thunk*) self)->th_args);
        Py_CLEAR(((thunk*) self)->th_kwargs);
    }

    return ((thunk*) self)->th_normal;
}

static PyTypeObject strict_type;

/* Strictly evaluate a thunk.
   return: A new reference. */
static PyObject *
strict_eval(PyObject *th)
{
    static PyObject *strict_name;
    PyObject *normal;
    PyObject *strict_method = NULL;

    if (PyObject_IsInstance(th, (PyObject*)  &thunk_type)) {
        if (!(th = _strict_eval_borrowed(th))) {
            return NULL;
        }
    }

    if (!(strict_name ||
          (strict_name = PyUnicode_FromString("__strict__")))) {
        return NULL;
    }
    if (!(strict_method = PyObject_GetAttr((PyObject*) Py_TYPE(th),
                                           strict_name))) {
        if (PyErr_ExceptionMatches(PyExc_AttributeError)) {
            PyErr_Clear();
            normal = th;
        }
        else {
            return NULL;
        }
    }
    else if (!(normal = PyObject_CallFunctionObjArgs(strict_method,
                                                     th,
                                                     NULL))) {
        Py_DECREF(strict_method);
        return NULL;
    }
    else {
        Py_XDECREF(strict_method);
    }

    Py_INCREF(normal);
    return normal;
}

static PyObject *
strict_new(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    static const char * const keywords[] = {"expr", NULL};
    PyObject *th;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "O:strict",
                                     (char**) keywords,
                                     &th)) {
        return NULL;
    }

    return strict_eval(th);
}

PyDoc_STRVAR(strict_doc,
             "Compute the normal form of an expression.\n"
             "\n"
             "Parameter\n"
             "---------\n"
             "expr : any\n"
             "    An expression of any type.\n"
             "\n"
             "Returns\n"
             "-------\n"
             "normal : any\n"
             "    The normal (computed) form of the expresson.\n"
             "\n"
             "Notes\n"
             "-----\n"
             "For objects that are not instances of ``thunk``, this is the\n"
             "identity. For ``thunks``s, this computes the expression\n");

static PyTypeObject strict_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "lazy._thunk.strict",                       /* tp_name */
    sizeof(PyObject),                           /* tp_basicsize */
    0,                                          /* tp_itemsize */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    strict_doc,                                 /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    (newfunc) strict_new,                       /* tp_new */
};

/* Thunk methods ----------------------------------------------------------- */

static void
thunk_free(thunk *self)
{
    PyObject_GC_Del(self);
}

static void
thunk_dealloc(thunk *self)
{
    PyObject_GC_UnTrack((PyObject*) self);
    Py_CLEAR(self->th_func);
    Py_CLEAR(self->th_args);
    Py_CLEAR(self->th_kwargs);
    Py_CLEAR(self->th_normal);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


/* Create a thunk without checking if `func` is a strict type.
   return: A new reference. */
static PyObject *
_thunk_new_no_check(PyTypeObject *cls,
                    PyObject *func,
                    PyObject *args,
                    PyObject *kwargs)
{
    thunk *self;

    if (!(self = (thunk*) cls->tp_alloc(cls, 0))) {
        return NULL;
    }

    Py_INCREF(func);
    self->th_func = func;

    Py_INCREF(args);
    self->th_args = args;

    Py_XINCREF(kwargs);
    self->th_kwargs = kwargs;

    self->th_normal = NULL;

    PyObject_Init((PyObject*) self, cls);
    return (PyObject*) self;
}

static PyObject *
_thunk_new_normal(PyTypeObject *cls, PyObject *normal)
{
    thunk *self;

    if (!(self = (thunk*) cls->tp_alloc(cls, 0))) {
        return NULL;
    }
    self->th_func = NULL;
    self->th_args = NULL;
    self->th_kwargs = NULL;
    self->th_normal = normal;
    Py_INCREF(normal);

    PyObject_Init((PyObject*) self, cls);
    return (PyObject*) self;
}

static PyObject *
inner_thunk_new(PyObject *cls, PyObject *func, PyObject *args, PyObject *kwargs)
{
    PyObject *ret;

    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_ValueError, "func must be callable");
        return NULL;
    }

    if (PyObject_IsInstance(func, (PyObject*) &PyType_Type) &&
        PyObject_IsSubclass(func, (PyObject*) &strict_type)) {
        /* Strict types get evaluated strictly. */
        if (PyTuple_GET_SIZE(args)) {
            /* There are args to apply to strict. */
            ret = PyObject_Call(func, args, kwargs);
        }
        else {
            /* There are no args to apply, return the strict type. */
            Py_INCREF(func);
            ret = func;
        }
    }
    else {
        ret = _thunk_new_no_check((PyTypeObject*) cls,
                                  func,
                                  args,
                                  kwargs);
    }
    return ret;
}

static PyObject *
LzThunk_New(PyObject *func, PyObject *args, PyObject *kwargs)
{
    return inner_thunk_new((PyObject*) &thunk_type, func, args, kwargs);
}

/* Create a thunk OR construct a strict type.
   return: A new reference. */
static PyObject *
thunk_new(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    PyObject *th_func;
    PyObject *th_args = NULL;
    Py_ssize_t nargs;
    Py_ssize_t n;
    PyObject *tmp;
    PyObject *ret;

    nargs = PyTuple_GET_SIZE(args);
    if (nargs < 1) {
        PyErr_SetString(PyExc_TypeError, "missing callable argument");
        return NULL;
    }

    th_func = PyTuple_GET_ITEM(args, 0);
    if (!(th_args = PyTuple_New(nargs - 1))) {
        return NULL;
    }
    for (n = 0;n < nargs - 1;++n) {
        tmp = PyTuple_GET_ITEM(args, n + 1);
        Py_INCREF(tmp);
        PyTuple_SET_ITEM(th_args, n, tmp);
    }
    ret = inner_thunk_new(cls, th_func, th_args, kwargs);
    Py_DECREF(th_args);
    return ret;
}


/* Binary operators --------------------------------------------------------- */
#define THUNK_BINOP(name, func)                                         \
    BINWRAPPER_FROM_FUNC(func, name)                                    \
    static PyObject *                                                   \
    thunk_ ## name(PyObject *a, PyObject *b)                            \
    {                                                                   \
        PyObject *arg;                                                  \
        PyObject *ret;                                                  \
        int instance_p;                                                 \
        if (!(arg = PyTuple_Pack(2, a, b))) {                           \
            return NULL;                                                \
        }                                                               \
        instance_p = PyObject_IsInstance(a, (PyObject*) &thunk_type);   \
        if (instance_p == -1) {                                         \
            Py_DECREF(arg);                                             \
            return NULL;                                                \
        }                                                               \
        ret = _thunk_new_no_check(Py_TYPE(instance_p ? a : b),          \
                                  LzBinary_ ## name,                    \
                                  arg,                                  \
                                  NULL);                                \
        Py_DECREF(arg);                                                 \
        return ret;                                                     \
    }

THUNK_BINOP(add, PyNumber_Add)
THUNK_BINOP(sub, PyNumber_Subtract)
THUNK_BINOP(mul, PyNumber_Multiply)

#if LZ_HAS_MATMUL
THUNK_BINOP(matmul, PyNumber_MatrixMultiply)
#endif

THUNK_BINOP(floordiv, PyNumber_FloorDivide)
THUNK_BINOP(truediv, PyNumber_TrueDivide)
THUNK_BINOP(rem, PyNumber_Remainder)
THUNK_BINOP(divmod, PyNumber_Divmod)
THUNK_BINOP(lshift, PyNumber_Lshift)
THUNK_BINOP(rshift, PyNumber_Rshift)
THUNK_BINOP(and, PyNumber_And)
THUNK_BINOP(xor, PyNumber_Xor)
THUNK_BINOP(or, PyNumber_Or)

/* Inplace operators -------------------------------------------------------- */

#define THUNK_INPLACE(name, func)                               \
    static PyObject *                                           \
    thunk_ ## name(PyObject *self, PyObject *other)             \
    {                                                           \
        PyObject *val;                                          \
        if (!(val = _strict_eval_borrowed(self))) {             \
            return NULL;                                        \
        }                                                       \
        return func(val, other);                                \
    }

THUNK_INPLACE(iadd, PyNumber_InPlaceAdd)
THUNK_INPLACE(isub, PyNumber_InPlaceSubtract)
THUNK_INPLACE(imul, PyNumber_InPlaceMultiply)

#if LZ_HAS_MATMUL
THUNK_INPLACE(thunk_imatmul, PyNumber_InPlaceMatrixMultiply)
#endif

THUNK_INPLACE(ifloordiv, PyNumber_InPlaceFloorDivide)
THUNK_INPLACE(itruediv, PyNumber_InPlaceTrueDivide)
THUNK_INPLACE(irem, PyNumber_InPlaceRemainder)
THUNK_INPLACE(ilshift, PyNumber_InPlaceLshift)
THUNK_INPLACE(irshift, PyNumber_InPlaceRshift)
THUNK_INPLACE(iand, PyNumber_InPlaceAnd)
THUNK_INPLACE(ixor, PyNumber_InPlaceXor)
THUNK_INPLACE(ior, PyNumber_InPlaceOr)

static PyObject *
thunk_ipower(PyObject *a, PyObject *b, PyObject *c)
{
    PyObject *val;

    if (PyObject_IsInstance(a, (PyObject*) &thunk_type)) {
        val = _strict_eval_borrowed(a);
        return PyNumber_InPlacePower(val, b, c);
    }
    else {
        val = _strict_eval_borrowed(b);
        return PyNumber_InPlacePower(a, val, c);
    }
}

/* Unary operators --------------------------------------------------------- */

#define THUNK_UNOP(name, func)                                          \
    UNARYWRAPPER_FROM_FUNC(func, name)                                  \
    static PyObject *                                                   \
    thunk_ ## name(PyObject *self)                                      \
    {                                                                   \
        PyObject *arg;                                                  \
        PyObject *ret;                                                  \
        if (!(arg = PyTuple_Pack(1, self))) {                           \
            return NULL;                                                \
        }                                                               \
        ret = _thunk_new_no_check(Py_TYPE(self), LzUnary_ ## name, arg, NULL); \
        Py_DECREF(arg);                                                 \
        return ret;                                                     \
    }

THUNK_UNOP(neg, PyNumber_Negative)
THUNK_UNOP(pos, PyNumber_Positive)
THUNK_UNOP(abs, PyNumber_Absolute)
THUNK_UNOP(inv, PyNumber_Invert)

/* Ternary operators ------------------------------------------------------- */

TERNARYWRAPPER_FROM_FUNC(PyNumber_Power, pow)

static PyObject *
thunk_power(PyObject *a, PyObject *b, PyObject *c)
{
    PyObject *arg;
    PyObject *ret;
    int instance_p;

    if (!(arg = PyTuple_Pack(3, a, b, c))) {
        return NULL;
    }

    if ((instance_p = PyObject_IsInstance(a, (PyObject*) &thunk_type)) == -1) {
        Py_DECREF(arg);
        return NULL;
    }
    ret = _thunk_new_no_check(Py_TYPE(instance_p ? a : b),
                              LzTernary_pow, arg,
                              NULL);
    Py_DECREF(arg);
    return ret;
}

/* Converters -------------------------------------------------------------- */

#define THUNK_STRICT_CONVERTER(name, type, default_, func)      \
    static type                                                 \
    thunk_ ## name(PyObject *self)                              \
    {                                                           \
        PyObject *val;                                          \
        if (!(val = _strict_eval_borrowed(self))) {             \
            return default_;                                    \
        }                                                       \
        return func(val);                                       \
    }

THUNK_STRICT_CONVERTER(long, PyObject*, NULL, PyNumber_Long)
THUNK_STRICT_CONVERTER(float, PyObject*, NULL, PyNumber_Float)
THUNK_STRICT_CONVERTER(index, PyObject*, NULL, PyNumber_Index)
THUNK_STRICT_CONVERTER(bool, int, -1, PyObject_IsTrue)

/* As number --------------------------------------------------------------- */

static PyNumberMethods thunk_as_number = {
    (binaryfunc) thunk_add,
    (binaryfunc) thunk_sub,
    (binaryfunc) thunk_mul,
    (binaryfunc) thunk_rem,
    (binaryfunc) thunk_divmod,
    (ternaryfunc) thunk_power,
    (unaryfunc) thunk_neg,
    (unaryfunc) thunk_pos,
    (unaryfunc) thunk_abs,
    (inquiry) thunk_bool,
    (unaryfunc) thunk_inv,
    (binaryfunc) thunk_lshift,
    (binaryfunc) thunk_rshift,
    (binaryfunc) thunk_and,
    (binaryfunc) thunk_xor,
    (binaryfunc) thunk_or,
    (unaryfunc) thunk_long,
    NULL,  /* reserved slot */
    (unaryfunc) thunk_float,
    (binaryfunc) thunk_iadd,
    (binaryfunc) thunk_isub,
    (binaryfunc) thunk_imul,
    (binaryfunc) thunk_irem,
    (ternaryfunc) thunk_ipower,
    (binaryfunc) thunk_ilshift,
    (binaryfunc) thunk_irshift,
    (binaryfunc) thunk_iand,
    (binaryfunc) thunk_ixor,
    (binaryfunc) thunk_ior,
    (binaryfunc) thunk_floordiv,
    (binaryfunc) thunk_truediv,
    (binaryfunc) thunk_ifloordiv,
    (binaryfunc) thunk_itruediv,
    (unaryfunc) thunk_index,

#if LZ_HAS_MATMUL
    (binaryfunc) thunk_matmul,
    (binaryfunc) thunk_imatmul,
#endif

};

/* As mapping -------------------------------------------------------------- */

static Py_ssize_t
thunk_len(PyObject *self)
{
    PyObject *val;

    if (!(val = _strict_eval_borrowed(self))) {
        return -1;
    }

    return PyObject_Size(val);
}

BINWRAPPER_FROM_FUNC(PyObject_GetItem, getitem)

static PyObject *
thunk_getitem(PyObject *self, PyObject *key)
{
    PyObject *arg;
    PyObject *ret;

    if (!(arg = Py_BuildValue("(OO)", self, key))) {
        return NULL;
    }
    ret = _thunk_new_no_check(Py_TYPE(self), LzBinary_getitem, arg, NULL);
    Py_DECREF(arg);
    return ret;
}

static int
thunk_setitem(PyObject *self, PyObject *key, PyObject *value)
{
    PyObject *val;

    if (!(val = _strict_eval_borrowed(self))) {
        return -1;
    }

    PyObject_SetItem(val, key, value);
    return 0;
}

static PyMappingMethods thunk_as_mapping = {
    (lenfunc) thunk_len,
    (binaryfunc) thunk_getitem,
    (objobjargproc) thunk_setitem,
};

/* Misc methods ------------------------------------------------------------ */

static Py_hash_t
thunk_hash(PyObject *self)
{
    PyObject *normal;

    if (!(normal = _strict_eval_borrowed(self))) {
        return -1;
    }

    return PyObject_Hash(normal);
}

static PyObject *
thunk_call(PyObject *self, PyObject *args, PyObject *kwargs)
{
    return _thunk_new_no_check(Py_TYPE(self), self, args, kwargs);
}

THUNK_STRICT_CONVERTER(repr, PyObject*, NULL, PyObject_Repr)
THUNK_STRICT_CONVERTER(str, PyObject*, NULL, PyObject_Str)

BINWRAPPER_FROM_FUNC(PyObject_GetAttr, getattr)

static PyObject *
thunk_getattro(PyObject *self, PyObject *name)
{
    PyObject *arg;
    PyObject *ret;

    if (!PyUnicode_CompareWithASCIIString(name, "__class__")) {
        if (!(arg = _strict_eval_borrowed(self))) {
            return NULL;
        }
        return PyObject_GetAttr(arg, name);
    }

    if (!(arg = PyTuple_Pack(2, self, name))) {
        return NULL;
    }
    ret = _thunk_new_no_check(Py_TYPE(self), LzBinary_getattr, arg, NULL);
    Py_DECREF(arg);
    return ret;
}

static int
thunk_setattro(PyObject *self, PyObject *name, PyObject *value)
{
    PyObject *normal;

    if (!(normal = _strict_eval_borrowed(self))) {
        return -1;
    }

    return PyObject_SetAttr(normal, name, value);
}

static int
thunk_traverse(thunk *self, visitproc visit, void *arg)
{
    if (self->th_func) {
        Py_VISIT(self->th_func);
    }
    if (self->th_args) {
        Py_VISIT(self->th_args);
    }
    if (self->th_kwargs) {
        Py_VISIT(self->th_kwargs);
    }
    if (self->th_normal) {
        Py_VISIT(self->th_normal);
    }
    return 0;
}

static int
thunk_clear(thunk *self)
{
    Py_CLEAR(self->th_func);
    Py_CLEAR(self->th_args);
    Py_CLEAR(self->th_kwargs);
    Py_CLEAR(self->th_normal);
    return 0;
}


THUNK_UNOP(iter, PyObject_GetIter)

static PyObject *
thunk_next(thunk *self)
{
    PyObject *tmp;
    PyObject *ret;

    if (!(tmp = _strict_eval_borrowed((PyObject*) self))) {
        return NULL;
    }

    if (!PyIter_Check(tmp)) {
        PyErr_Format(PyExc_TypeError,
                     "'%s' object is not an iterator'",
                     Py_TYPE(tmp)->tp_name);
        return NULL;
    }

    if (!(tmp = PyIter_Next(tmp)) || PyErr_Occurred()) {
        return NULL;
    }

    ret = thunk_fromvalue(Py_TYPE(self), tmp);
    Py_DECREF(tmp);
    return ret;
}

/* Rich compare helpers ---------------------------------------------------- */

#define THUNK_CMPOP(name, op)                                           \
    static inline PyObject *                                            \
    thunk_ ## name(PyObject *self, PyObject *other)                     \
    {                                                                   \
        return PyObject_RichCompare(self, other, op);                   \
    }                                                                   \
    BINWRAPPER_FROM_FUNC(thunk_ ## name, name)

THUNK_CMPOP(lt, Py_LT)
THUNK_CMPOP(le, Py_LE)
THUNK_CMPOP(eq, Py_EQ)
THUNK_CMPOP(ne, Py_NE)
THUNK_CMPOP(gt, Py_GT)
THUNK_CMPOP(ge, Py_GE)

static PyObject *
thunk_richcmp(thunk *self, PyObject *other, int op)
{
    PyObject *func;
    PyObject *arg;
    PyObject *ret;

    if (!(arg = PyTuple_Pack(2, self, other))) {
        return NULL;
    }

    switch(op) {
    case Py_LT:
        func = LzBinary_lt;
        break;
    case Py_LE:
        func = LzBinary_le;
        break;
    case Py_EQ:
        func = LzBinary_eq;
        break;
    case Py_NE:
        func = LzBinary_ne;
        break;
    case Py_GT:
        func = LzBinary_gt;
        break;
    case Py_GE:
        func = LzBinary_ge;
        break;
    default:
      Py_DECREF(arg);
      PyErr_BadInternalCall();
      return NULL;
    }

    ret = _thunk_new_no_check(Py_TYPE(self), func, arg, NULL);
    Py_DECREF(arg);
    return ret;
}

/* Extra methods ----------------------------------------------------------- */

PyDoc_STRVAR(thunk_fromvalue_doc,
             "Create a thunk that wraps a single value.\n"
             "\n"
             "Parameters\n"
             "----------\n"
             "value : any\n"
             "    The value to wrap\n"
             "\n"
             "Returns\n"
             "-------\n"
             "th : thunk\n"
             "    The boxed version of ``value``.\n");

/* Create a thunk that wraps a single value. */
static PyObject *
thunk_fromvalue(PyTypeObject *cls, PyObject *value)
{
    if (PyObject_IsInstance(value, (PyObject*) &PyType_Type) &&
        PyObject_IsSubclass(value, (PyObject*) &strict_type)) {
        Py_INCREF(value);
        return value;
    }
    if (PyObject_IsSubclass((PyObject*) Py_TYPE(value),
                            (PyObject*) &thunk_type)) {
        /* thunk.fromvalue of a thunk is the identity */
        Py_INCREF(value);
        return value;
    }

    return  _thunk_new_normal(cls, value);
}

static PyObject *
LzThunk_FromValue(PyObject *value)
{
    return thunk_fromvalue(&thunk_type, value);
}

PyDoc_STRVAR(get_children_doc,
             "Get the children of a thunk.\n"
             "\n"
             "Parameters\n"
             "----------\n"
             "th : thunk\n"
             "    The thunk to get the children of.\n"
             "\n"
             "Returns\n"
             "-------\n"
             "children : tuple\n"
             "    The children of ``th``. This will either be a tuple of:\n"
             "        (func, args, kwargs)\n"
             "    or\n"
             "        (normal,)\n"
             "    The first case is when the thunk has never been computed.\n"
             "    The second case is when the thunk has been computed. By\n"
             "    this point we no longer have the func, args or kwargs.\n");

static PyObject *
LzThunk_GetChildren(PyObject *th)
{
    thunk *asthunk;
    PyObject *kwargs;
    PyObject *ret;

    if (!PyObject_IsInstance(th, (PyObject*)  &thunk_type)) {
        PyErr_SetString(
            PyExc_TypeError,
            "get_children expected argument of type thunk");
        return NULL;
    }
    asthunk = (thunk*) th;

    if (asthunk->th_normal) {
        return PyTuple_Pack(1, asthunk->th_normal);
    }

    if (!(kwargs = (asthunk->th_kwargs) ? asthunk->th_kwargs : PyDict_New())) {
        return NULL;
    }

    ret = PyTuple_Pack(3,
                       asthunk->th_func,
                       asthunk->th_args,
                       kwargs);
    Py_DECREF(kwargs);
    return ret;
}

static PyObject *
get_children(PyObject *self, PyObject *th)
{
    return LzThunk_GetChildren(th);
}

PyMethodDef thunk_methods[] = {
    {"fromvalue",
     (PyCFunction) thunk_fromvalue,
     METH_CLASS | METH_O,
     thunk_fromvalue_doc},
    {NULL},
};

/* thunk definition -------------------------------------------------------- */

PyDoc_STRVAR(thunk_doc,
             "A deferred computation.\n"
             "\n"
             "Parameters\n"
             "----------\n"
             "func : callable\n"
             "    The function to delay calling.\n"
             "*args : any\n"
             "    The positional arguments to pass to ``func``.\n"
             "**kwargs : any\n"
             "    The keyword arguments to pass to ``func``.\n");

static PyTypeObject thunk_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "lazy._thunk.thunk",                        /* tp_name */
    sizeof(thunk),                              /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor) thunk_dealloc,                 /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc) thunk_repr,                      /* tp_repr */
    &thunk_as_number,                           /* tp_as_number */
    0,                                          /* tp_as_sequence */
    &thunk_as_mapping,                          /* tp_as_mapping */
    (hashfunc) thunk_hash,                      /* tp_hash */
    (ternaryfunc) thunk_call,                   /* tp_call */
    (reprfunc) thunk_str,                       /* tp_str */
    (getattrofunc) thunk_getattro,              /* tp_getattro */
    (setattrofunc) thunk_setattro,              /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC,                         /* tp_flags */
    thunk_doc,                                  /* tp_doc */
    (traverseproc) thunk_traverse,              /* tp_traverse */
    (inquiry) thunk_clear,                      /* tp_clear */
    (richcmpfunc) thunk_richcmp,                /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    (getiterfunc) thunk_iter,                   /* tp_iter */
    (iternextfunc) thunk_next,                  /* tp_iternext */
    thunk_methods,                              /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    &PyBaseObject_Type,                         /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    (allocfunc) PyType_GenericAlloc,            /* tp_alloc */
    (newfunc) thunk_new,                        /* tp_new */
    (freefunc) thunk_free,                      /* tp_free */
};

/* Module level ------------------------------------------------------------ */

PyDoc_STRVAR(module_doc,"A defered computation.");

static PyMethodDef module_methods[] = {
    {"get_children",
     (PyCFunction) get_children,
     METH_O,
     get_children_doc},
    {NULL},
};

static LzExported exported_symbols = {
    LzThunk_New,
    LzThunk_FromValue,
    LzThunk_GetChildren,
};

static struct PyModuleDef _thunk_module = {
    PyModuleDef_HEAD_INIT,
    "lazy._thunk",
    module_doc,
    -1,
    module_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyDoc_STRVAR(operator_doc,"Operators used by thunks.");

static struct PyModuleDef _operator_module = {
    PyModuleDef_HEAD_INIT,
    "lazy._thunk.operator",
    operator_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__thunk(void)
{
    PyObject *m;
    PyObject *operator;
    PyObject *symbols;
    PyTypeObject *types[] = {&unarywrapper_type,
                             &binwrapper_type,
                             &ternarywrapper_type,
                             &strict_type,
                             &thunk_type,
                             NULL};
    size_t n = 0;

    if (!(symbols = PyCapsule_New(&exported_symbols,
                                  "lazy._thunk._exported_symbols",
                                  NULL))) {
        return NULL;
    }

    while (types[n]) {
        if (PyType_Ready(types[n])) {
            Py_DECREF(symbols);
            return NULL;
        }
        ++n;
    }

    if (!(m = PyModule_Create(&_thunk_module))) {
        Py_DECREF(symbols);
        return NULL;
    }

    n = PyObject_SetAttrString(m, "_exported_symbols", symbols);
    Py_DECREF(symbols);
    if (n) {
        Py_DECREF(m);
        return NULL;
    }

    if (!(operator = PyModule_Create(&_operator_module))) {
        Py_DECREF(m);
        return NULL;
    }

#define ADD_OPERATOR(prefix, name)                                      \
    if (PyObject_SetAttrString(operator, STR(name), prefix ## name)) {  \
        Py_DECREF(operator);                                            \
        Py_DECREF(m);                                                   \
        return NULL;                                                    \
    }
#define ADD_BINARY_OPERATOR(name) ADD_OPERATOR(LzBinary_, name)
#define ADD_UNARY_OPERATOR(name) ADD_OPERATOR(LzUnary_, name)

    ADD_BINARY_OPERATOR(getitem)
    ADD_BINARY_OPERATOR(getattr)
    ADD_BINARY_OPERATOR(add)
    ADD_BINARY_OPERATOR(sub)
    ADD_BINARY_OPERATOR(mul)
    ADD_BINARY_OPERATOR(rem)
    ADD_BINARY_OPERATOR(divmod)
    ADD_BINARY_OPERATOR(lshift)
    ADD_BINARY_OPERATOR(rshift)
    ADD_BINARY_OPERATOR(and)
    ADD_BINARY_OPERATOR(xor)
    ADD_BINARY_OPERATOR(or)
    ADD_BINARY_OPERATOR(gt)
    ADD_BINARY_OPERATOR(ge)
    ADD_BINARY_OPERATOR(lt)
    ADD_BINARY_OPERATOR(le)
    ADD_BINARY_OPERATOR(eq)
    ADD_BINARY_OPERATOR(ne)
#if LZ_HAS_MATMUL
    ADD_BINARY_OPERATOR(matmul)
#endif
    ADD_UNARY_OPERATOR(neg)
    ADD_UNARY_OPERATOR(pos)
    ADD_UNARY_OPERATOR(abs)
    ADD_UNARY_OPERATOR(inv)
    ADD_OPERATOR(LzTernary_, pow)
#undef ADD_BINARY_OPERATOR
#undef ADD_UNARY_OPERATOR
#undef ADD_OPERATOR

    n = PyObject_SetAttrString(m, "operator", operator);
    Py_DECREF(operator);
    if (n) {
        Py_DECREF(m);
        return NULL;
    }

    if (PyObject_SetAttrString(m, "thunk", (PyObject*) &thunk_type)) {
        Py_DECREF(m);
        return NULL;
    }

    if (PyObject_SetAttrString(m, "strict", (PyObject*) &strict_type)) {
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
