%if BUILD_SHARED
%define REF(name) rel name wrt ..got
%define REF_DATA(name) rel name
%else
%define REF(name) name
%define REF_DATA(name) name
%endif
