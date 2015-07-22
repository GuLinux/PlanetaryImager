#ifndef D_PTR_H
#define D_PTR_H
#include <memory>

#define D_PTR class Private; friend class Private; const std::unique_ptr<Private> d;
#define dpointer(...) d{new Private{__VA_ARGS__}}

#endif