src = [
  'open_monitor.cpp',
]

deps = [
    gst_dep,
    gstsdp_dep,
    gstwebrtc_dep,
    libsoup_dep,
    json_glib_dep,
    glibmm_dep,
    luawrapper,
    boost_program_options_dep
]

babyfoon = executable('open_monitor', src,
  dependencies: deps,
  install : true,
)
