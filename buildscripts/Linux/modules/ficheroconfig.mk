#See linux_base.mk for invocation options. 
progname:=ficheroconfig
exename:= fconfig
dependencies=$(hpath)*.h $$(dir $(mainfile))/*
include ./paths.mk
include ./linux_base.mk
