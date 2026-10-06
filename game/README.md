# Local game reference

`installation` is an ignored absolute symlink to
`/home/vibeshibe/Games/Transport Fever 3`. Moving this project does not break it.
Create it in a fresh checkout with:

```sh
rtk proxy ln -s "/home/vibeshibe/Games/Transport Fever 3" game/installation
```

Do not run recursive scans that follow the symlink. While the project is staged
inside the game directory, following it recursively would revisit this project.
