function(add_lab name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE core)
endfunction()

add_lab(lab1)
add_lab(lab2)
