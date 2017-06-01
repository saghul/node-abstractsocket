{
  'targets': [
    {
      'target_name': 'abstract_socket',
      'defines': [ '_GNU_SOURCE=1' ],
      'sources': [ 'src/abstract_socket.cc' ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")'
      ]
    }
  ]
}
