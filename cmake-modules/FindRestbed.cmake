set( restbed_FOUND TRUE )
set( restbed_DISTRIBUTION "/usr/local" )

set( restbed_INCLUDE "${restbed_DISTRIBUTION}/include" )
set( restbed_LIBRARY "${restbed_DISTRIBUTION}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}restbed${CMAKE_SHARED_LIBRARY_SUFFIX}" )

message( STATUS "${Green}Found Restbed include at: ${restbed_INCLUDE}" )
message( STATUS "${Green}Found Restbed library at: ${restbed_LIBRARY}" )