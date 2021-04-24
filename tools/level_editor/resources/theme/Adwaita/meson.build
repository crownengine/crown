scss_files = files([
  '_colors-public.scss',
  '_colors.scss',
  '_common.scss',
  '_drawing.scss',
])

theme_variants = [
  'dark',
]

theme_deps += custom_target('Adwaita',
                            input: 'gtk-contained.scss',
                            output: 'gtk-contained.css',
                            command: [
                              sassc, sassc_opts, '@INPUT@', '@OUTPUT@',
                            ],
                            depend_files: scss_files,
                            build_by_default: true)

foreach variant: theme_variants
  theme_deps += custom_target('Adwaita-' + variant,
                              input: 'gtk-contained-@0@.scss'.format(variant),
                              output: 'gtk-contained-@0@.css'.format(variant),
                              command: [
                                sassc, sassc_opts, '@INPUT@', '@OUTPUT@',
                              ],
                              depend_files: scss_files,
                              build_by_default: true)
endforeach
