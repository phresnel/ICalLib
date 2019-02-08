#ifndef XVARIANT_HH_INCLUDED_20190208
#define XVARIANT_HH_INCLUDED_20190208

#include <variant>
#include <utility>

// variant related
using std::variant;
using std::holds_alternative;
using std::get;
using std::get_if;

// Variant of variant whose `xvariant(T&& t)` is not implicit.
// TODO: review noexcepts
template<class... Types>
class xvariant : public variant<Types...> {
public:
        using variant = variant<Types...>;

        constexpr
        xvariant()
                : variant()
        {}

        constexpr xvariant(const xvariant& other) : variant(other) {}

        constexpr xvariant(xvariant&& other)
                : variant(other)
        {}

        template <class T>
        constexpr
        explicit // <-- this is the one and only reason xvariant exists.
        xvariant(T&& t)
                : variant(std::forward<T>(t))
        {}

        template <class T, class ...Args>
        constexpr
        explicit
        xvariant(std::in_place_type_t<T> ipt, Args&&... args)
                : variant(ipt, std::forward<Args>(args)...)
        {}

        template <class T, class U, class ...Args>
        constexpr
        explicit
        xvariant(
                std::in_place_type_t<T> ipt,
                std::initializer_list<U> il, Args&&... args
        ) : variant(ipt, std::forward<Args>(args)...)
        {}

        template <std::size_t I, class ...Args>
        constexpr
        explicit
        xvariant(
                std::in_place_index_t<I> ipi,
                Args&&... args
        ) : variant(ipi, std::forward<Args>(args)...)
        {}

        template <std::size_t I, class U, class ...Args>
        constexpr
        explicit
        xvariant(
                std::in_place_index_t<I> ipi,
                std::initializer_list<U> il,
                Args&&... args
        ) : variant(ipi, il, std::forward<Args>(args)...)
        {}

        constexpr
        xvariant& operator=(const xvariant& rhs) {
                variant::operator= (rhs);
                return *this;
        }

        constexpr
        xvariant& operator=(xvariant&& rhs)
        noexcept(noexcept(variant::operator= (rhs)))
        {
                variant::operator= (rhs);
                return *this;
        }

        template <class T>
        xvariant& operator=(T&& rhs)
        noexcept(noexcept(variant::operator= (std::forward<T>(rhs))))
        {
                variant::operator= (std::forward<T>(rhs));
                return *this;
        }
};

template <class Visitor, typename ...XVariants>
// TODO: Needs enable_if upon xvariant
auto visit(Visitor&& vis, XVariants&& ...xvars) -> void {
        using variant = variant<Types...>;

        using std::visit;

        // TODO: Either cast, or copy to a fresh std::variant

        /*std::visit(
                vis,
        );*/
        // TODO: Proper return [type]
}

template<typename T, typename ...VTypes>
inline
optional<T> get_opt(xvariant<VTypes...> const &v) {
        auto p = get_if<T>(&v);
        if (!p) return nullopt;
        return *p;
}

template<typename T, typename ...VTypes>
inline
optional<T> get_opt(variant<VTypes...> const &v) {
        auto p = get_if<T>(&v);
        if (!p) return nullopt;
        return *p;
}

#endif //XVARIANT_HH_INCLUDED_20190208
