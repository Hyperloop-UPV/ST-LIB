#pragma once
#include "C++Utilities/CppUtils.hpp"

template<class... Types>
struct TemplatePack{
    using Pack = Types...;
};

template<class FilterType, class... ToFilter>
struct TemplatePackFilter;

template<class FilterType, class First, class... ToFilter> requires requires { requires !is_same<FilterType, First>::value; }
struct TemplatePackFilter<FilterType, First, ToFilter...>{
    using FilteredPack = typename TemplatePackFilter<FilterType, ToFilter...>::FilteredPack;
};

template<class FilterType, class... ToFilter>
struct TemplatePackFilter<FilterType, FilterType, ToFilter...>{
    using FilteredPack = TemplatePack<FilterType, typename TemplatePackFilter<FilterType, ToFilter...>::FilteredPack>::Pack;
};

template<class FilterType>
struct TemplatePackFilter<FilterType>{
    using FilteredPack = TemplatePack<>::Pack;
};
