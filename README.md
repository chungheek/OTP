# One Time Pad (OTP)

OTP is an application that can encrypt and decrypt a plaintext and ciphertext respectively.

Make sure port is the same for decryption/encryption module respectively.

**NOTE**: Plain text files can only contain capitalized letters and spaces

---

Commands
---
- `./dec_server {ds_port}`
- `./dec_client {ciphertextFile} {keyFile} {ds_port}`

- `./enc_server {es_port}`
- `./enc_client {plaintextFile} {keyFile} {es_port}`

---
In order to run
---
- run `Make`
- run `./enc_server {port}`
- run `./dec_server {port}`
- In order to encrypt:
	- run `./enc_client {plaintextFile} {keyFile} {encryption server port}`
- In order to decrypt:
	- run `./dec_client {ciphertextFile} {keyFile} {decryption server port}`

Sample key `mykey` is provided that can be overwritten

Use `Make clean` to remove executables or after code modification
