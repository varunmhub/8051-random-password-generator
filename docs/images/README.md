# Images

Drop the project photographs and output captures in this folder, then reference them from `README.md` or `docs/PROJECT_REPORT.md`.

## Suggested files

| Filename | What it should show |
|---|---|
| `circuit-diagram.png` | Schematic of the AT89S52, LCD, button and power supply |
| `block-diagram.png` | System block diagram |
| `hardware-full.jpg` | The complete assembled board, powered on |
| `hardware-closeup.jpg` | Microcontroller, crystal and reset circuitry in detail |
| `output-idle.jpg` | LCD showing the `Press Key` idle prompt |
| `output-password-1.jpg` | LCD showing a generated password, e.g. `A0hP$wdL` |
| `output-password-2.jpg` | A second generated password, e.g. `Q&vdKarZ` |
| `keil-build.png` | Keil µVision build output with zero errors and the HEX file created |

## Referencing an image

Once a file is uploaded, embed it with a relative path:

```markdown
![Assembled hardware](docs/images/hardware-full.jpg)
```

From inside `docs/PROJECT_REPORT.md`, use the shorter relative path instead:

```markdown
![Assembled hardware](images/hardware-full.jpg)
```

## Tips

- Photograph the LCD straight on, with the backlight on and room lights dimmed, so the characters are legible.
- Keep photos under about 2 MB each so the repository stays light to clone.
- Prefer `.png` for diagrams and screenshots, `.jpg` for photographs.
