function(add_cli name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE core)
endfunction()

add_cli(cli)
