include "./premake_customizations.lua"

workspace "Nirnia"
	location ("Build/" .. _ACTION)
	architecture "x86_64"
	
	startproject "Nirnia"

	configurations
	{
		"Debug",
		"Release",
	}

	workspace_files 
	{
		".editorconfig"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Hazel/Hazel"
	include "Hazel/Hazel/vendor/GLFW"
	include "Hazel/Hazel/vendor/Glad"
	include "Hazel/Hazel/vendor/imgui"	

group ""
	include "Nirnia"
