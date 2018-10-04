#include <cstdint>
#include <new>

void *operator new(std::size_t) noexcept;

void *operator new[](std::size_t) noexcept;

void operator delete(void *) noexcept;

void operator delete[](void *) noexcept;

void *operator new(std::size_t, const std::nothrow_t &) noexcept;

void *operator new[](std::size_t, const std::nothrow_t &) noexcept;

void operator delete(void *, const std::nothrow_t &) noexcept;

void operator delete[](void *, const std::nothrow_t &) noexcept;

void operator delete(void *m, std::size_t) noexcept;

void operator delete[](void *m, std::size_t) noexcept;
