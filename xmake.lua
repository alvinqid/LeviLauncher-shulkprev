set_project("ShulkerboxPreview")
set_version("1.0.0")

set_languages("c99", "cxx23")

add_rules("mode.release")

add_cxflags(
    "-O2",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-fno-rtti",
    "-fno-exceptions",
    "-fomit-frame-pointer",
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables",
    "-flto",
    "-w"
)

add_cflags(
    "-O2",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-flto",
    "-w"
)

add_ldflags(
    "-Wl,--gc-sections",
    "-Wl,--strip-all",
    "-Wl,--exclude-libs,ALL",
    "-Wl,--no-eh-frame-hdr",
    "-flto",
    "-s",
    {force = true}
)

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")
add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("glm")
add_requires("preloader_android 0.1.13")

target("ShulkerboxPreview")
    set_kind("shared")
    add_files("src/**.cpp")
    add_headerfiles("src/**.hpp")
    add_includedirs("src", {public = true})
    add_packages("preloader_android", "glm")
    add_links("log")