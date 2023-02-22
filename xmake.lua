add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_languages("c++17")
set_warnings("everything")

add_includedirs("include", "dns")

target("light_socks", {
	kind = "shared",
	files = "src/*.cpp",
})
