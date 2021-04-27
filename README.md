# One Time Pad (OTP)

OTP is an application that can encrypt and decrypt a plaintext and ciphertext respectively.

Make sure port is the same for decryption/encryption module respectively

---

Commands
---
- `./dec_server {port}`
- `./dec_client {ciphertextFile} {keyFile} {port}`

- `./enc_server {port}`
- `./enc_client {plaintextFile} {keyFile} {port}`

---
In order to run
---
- run `Make`
- run `./enc_server {port}`
- run `./dec_server {port}`
- In order to encrypt:
	- run `./enc_client {plaintextFile} {keyFile} {port}`
- In order to decrypt:
	- run `./dec_client {ciphertextFile} {keyFile} {port}`

Sample key `mykey` is provided that can be overwritten