// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC.

// 
// Security-related functions for VELY and run-time
// (getting db credentials, encryption etc)
//


#include "vely.h"
#include <openssl/err.h>

// Prototypes
void vely_sec_err (const char *err);


//
// CSPRNG random data generator. buf is the pre-allocated buffer,
// num is the space available. Returns 1 if okay, otherwise couldn't do it
//
int vely_RAND_bytes(unsigned char *buf, int num) 
{
    VV_TRACE("");
    return RAND_bytes (buf, num);
}

//
// Load crypto cipher and digest algos on startup, called from generated stub for main
//
void vely_sec_load_algos(void) 
{
    VV_TRACE("");
    OpenSSL_add_all_algorithms(); // so algos are available when asked for
}

// 
// Errors out.
// 'err' is vely messsage, which is supplemented with crypto-provider's message
// it doesn't return, rather ends with fatal error
//
void vely_sec_err (const char *err)
{
    VV_TRACE("");
    BIO *bio = BIO_new (BIO_s_mem ()); // this can fail with NULL
    if (bio == NULL) vely_report_error ("%s [could not obtain error message]", err);
    ERR_print_errors (bio); // cannot fail
    char *buf = NULL;
    size_t len = BIO_get_mem_data (bio, &buf); // returns buf, len
    // to get the earliest error: ERR_error_string(ERR_get_error(), NULL)
    vely_report_error ("%s [%.*s]", err, (int)len, buf);
    BIO_free (bio); // never really gets here, but no matter, the program is gone
}


// 
// Return hash value of 'val'.
// 'digest_name' is the name of digest algorithm, see 'openssl list cipher-algorithm'
// The result is a zero-terminated string in hex representation - XX bytes + zero byte.
// XX can be 64, 96 or some other number depending on digest.
// if 'binary' is true, the output is binary and outlen is the length of output binary data
//
char *vely_hash_data( const char *val, const char *digest_name, bool binary, num *outlen )
{
    VV_TRACE("");
    unsigned char hash[EVP_MAX_MD_SIZE + 1];

    EVP_MD_CTX *mdctx;
    if ((mdctx = EVP_MD_CTX_new()) == NULL) vely_sec_err ("Cannot allocate digest context");
    const EVP_MD *md = EVP_get_digestbyname(digest_name);
    if (md == NULL) vely_sec_err ("Unknown digest");
    EVP_MD_CTX_init(mdctx);
    EVP_DigestInit_ex(mdctx, md, NULL);

    num msg_length = strlen( val );

    char *out = (char *) vely_malloc( binary ? (EVP_MAX_MD_SIZE + 1) : ((EVP_MAX_MD_SIZE + 1)*2)+1) ;

    char *p = out;
    EVP_DigestUpdate(mdctx, val,(unsigned int) msg_length);
    EVP_DigestFinal_ex(mdctx,binary?(unsigned char*)out:hash, (unsigned int*)&msg_length);

    // if binary the output is done, set the output length
    if (binary) { if (outlen != NULL) *outlen = msg_length; return out; }

    // This is if not binary, convert hash to out, which is hexstring
    num i;
    for ( i = 0; i < msg_length; i++, p += 2 ) 
    {
        VV_HEX_FROM_BYTE (p, (unsigned int)hash[i]);
    }
    if (outlen != NULL) *outlen = (p-out); // length of output
    *p = 0;
    return out;
}


// 
// Return KD (key derivation) of 'val'. Uses PBKDF2. val_len is length of val or -1 if null-terminated.
// 'digest_name' is the name of digest algorithm, see 'openssl list cipher-algorithm'
// iter_count is num of iterations, salt/salt_len is salt and its length (0 for salt_len if null-terminated)
// key_len is the desired length of output (though string is not of that length)
// if binary is true, the output is binar of key_len length
// The result is a zero-terminated string in hex representation.
//
char *vely_derive_key( const char *val, num val_len, const char *digest_name, num iter_count, const char *salt, num salt_len, num key_len, bool binary )
{
    VV_TRACE("");
    const EVP_MD *dgst = NULL;
    // +1 as 0 is placed after each
    unsigned char *key = vely_malloc (key_len + 1);
#if OPENSSL_VERSION_MAJOR  >= 3
// in OpenSSL3, only if the implementation of digest exists, it will be non-NULL, while EV_get_digestbyname may return non-NULL
// even if not existing
    dgst = EVP_MD_fetch(NULL, digest_name, NULL);
#else
    dgst = EVP_get_digestbyname(digest_name);
#endif
    if(!dgst) { 
        VV_TRACE("Digest name [%s]", digest_name);
        vely_sec_err ("Digest not found");
    }
    if (iter_count == -1) iter_count=1000; 
    if (salt != NULL && salt_len == 0) salt_len = strlen (salt);
    if (!PKCS5_PBKDF2_HMAC (val, val_len==-1?strlen(val):(size_t)val_len, (const unsigned char *)salt, salt_len, iter_count, dgst, key_len, key))
    {
        vely_sec_err ("Cannot generate key");
    }
    if (binary) 
    {
        key[key_len] = 0;
        return (char*)key;
    }
    else
    {
        char *out = (char *) vely_malloc( sizeof(char) * (((key_len + 1)*2)+1) );
        char *p = out;
        num i;
        for ( i = 0; i < key_len; i++, p += 2 ) 
        {
            VV_HEX_FROM_BYTE (p, (unsigned int)key[i]);
        }
        *p = 0;
        vely_free (key);
        return out;
    }

}


// 
// Produce a key and IV out of password
// Then use this key/IV to actually encrypt or decrypt.
// 'password' is the password to encrypt with. 'e_ctx' is encrypt context
// (used to encrypt) and 'd_ctx' is decrypt context (used to decrypt).
// 'salt' is the salt used. salt_len is the length of salt, if 0 then strlen(salt), also if salt is NULL, salt_len must be 0.
// iter_count is the number of iterations in generating key and IV, if -1 then it's 1000
// 'cipher' is the cipher algorithm name. This is something one can see with 'openssl list cipher-algorithm'
// Vely does not restrict what this may be - which depends on what providers are present on the system,
// it only supplies the default, which is currently aes256.
// 'digest_name' is the digest algo name. This is something one can see with 'openssl list digest-algorithm'
// Vely does not restrict what this may be - which depends on what providers are present on the system,
// it only supplies the default, which is currently sha256
// Either e_ctx or d_ctx can be NULL (if we're only encrypting or decrypting).
// Returns 0 if cannot produce the context, 1 if okay.
//
num vely_get_enc_key(const char *password, const char *salt, num salt_len, num iter_count, EVP_CIPHER_CTX *e_ctx, 
             EVP_CIPHER_CTX *d_ctx, const char *cipher_name, const char *digest_name)
{
    VV_TRACE("");


    const EVP_CIPHER *cipher;
    const EVP_MD *dgst = NULL;
    // +1 as 0 is placed after each
    unsigned char key[EVP_MAX_KEY_LENGTH+EVP_MAX_IV_LENGTH+1];

#if OPENSSL_VERSION_MAJOR  >= 3
// in OpenSSL3, only if the implementation of cipher exists, it will be non-NULL, while EV_get_cipherbyname may return non-NULL
// even if not existing
    cipher = EVP_CIPHER_fetch(NULL, cipher_name, NULL);
#else
    cipher = EVP_get_cipherbyname(cipher_name);
#endif
    if(!cipher) { 
        VV_TRACE("Cipher name [%s]", cipher_name);
        vely_sec_err ("Cipher not found");
    }

#if OPENSSL_VERSION_MAJOR  >= 3
// in OpenSSL3, only if the implementation of digest exists, it will be non-NULL, while EV_get_digestbyname may return non-NULL
// even if not existing
    dgst = EVP_MD_fetch(NULL, digest_name, NULL);
#else
    dgst = EVP_get_digestbyname(digest_name);
#endif
    if(!dgst) { 
        VV_TRACE("Digest name [%s]", digest_name);
        vely_sec_err ("Digest not found");
    }

    if (salt != NULL && salt_len == 0) salt_len = strlen (salt);
    if (iter_count == -1) iter_count=1000; 

    int key_len = EVP_CIPHER_key_length(cipher);
    int iv_len = EVP_CIPHER_iv_length(cipher);

    if (!PKCS5_PBKDF2_HMAC (password, strlen(password), (const unsigned char *)salt, salt_len, iter_count, dgst, key_len + iv_len, key))
    {
        vely_sec_err ("Cannot convert password to keyring");
    }

    // This is the reason for encrypt/decrypt in new-key: if we didn't have that
    // then these two would have to repeat for every encryption/decryption, which would
    // make it slower
    if (e_ctx != NULL)
    {
        EVP_CIPHER_CTX_init(e_ctx);
#if OPENSSL_VERSION_MAJOR  >= 3
        if (EVP_EncryptInit_ex2(e_ctx, cipher, key, key+key_len, NULL) !=1) 
#else
        if (EVP_EncryptInit_ex(e_ctx, cipher, NULL, key, key+key_len) !=1) 
#endif
        {
            vely_sec_err ("Cannot encrypt");
        }
    }
    if (d_ctx != NULL)
    {
        EVP_CIPHER_CTX_init(d_ctx);
#if OPENSSL_VERSION_MAJOR  >= 3
        if (EVP_DecryptInit_ex2(d_ctx, cipher, key, key+key_len, NULL) != 1) 
#else
        if (EVP_DecryptInit_ex(d_ctx, cipher, NULL, key, key+key_len) != 1) 
#endif
        {
            vely_sec_err ("Cannot decrypt");
        }
    }

    return 1;
             
}

// 
// Encrypt string 'plaintext' and return encrypted value, with output parameter 
// 'len' being the length of this encrypted value returned by the function. *len is
// the length of data to encrypt, and also the output length after being done (excluding zero byte at the end).
// is_binary is 1 if the encrypted value is binary and not hex-string, otherwise 0.
// We allocate new memory for encrypted value - caller must de-allocate (this is vely_
// allocation, so you may just leave it to be collected by VELY memory garbage collector.
// 'e' is the encryption context, produced as e_ctx in vely_get_enc_key().
// iv is the nonce (IV), the length of which should be sufficient for the algorithm
// The maximum length of encrypted data is 2*(input_len+AES_BLOCK_SIZE)+1. Note that 
// AES_BLOCK_SIZE is always 16 bytes. So the maximum is 2*(input_len+16)+1 for the purposes
// of sizing.
//
char *vely_encrypt(EVP_CIPHER_CTX *e, const unsigned char *plaintext, num *len, num is_binary, unsigned char *iv)
{
    assert (*len>=0);

    /* maximum length for encrypted value is *len + BLOCK_SIZE -1 
      and block size depends on cipher with EVP_MAX_IV_LENGTH being the max for all if this ever fails*/
    int p_len = *len + EVP_CIPHER_CTX_block_size (e);
    int f_len = 0;
    unsigned char *ciphertext = vely_malloc(p_len+1); 
           
    // reuse the values already set, so same context can be reused for multiple encryptions
    // except nonce (IV), which is unique each time
    // if iv is NULL, Vely allows that only if cache is not used. In that case, the password is
    // computed each time and salt must be different each time, so new iv is generated every time.
#if OPENSSL_VERSION_MAJOR  >= 3
    EVP_EncryptInit_ex2(e, NULL, NULL, iv, NULL);
#else
    EVP_EncryptInit_ex(e, NULL, NULL, NULL, iv);
#endif
                
    /* update ciphertext, p_len is filled with the length of encrypted text
      len is the size of plaintext in bytes */
    EVP_EncryptUpdate(e, ciphertext, &p_len, plaintext, *len);
                     
    /* add to ciphertext the final remaining bytes */
    EVP_EncryptFinal_ex(e, ciphertext+p_len, &f_len);
                          
    *len = p_len + f_len;

    if (is_binary == 0)
    {
        //
        // Make encrypted text as hex-string for db storage
        //
        char *hex_ciphertext = vely_malloc(2*(*len)+1); 
        num i;
        //
        // Update progress by 2 bytes in hex-string mode
        //
        num tot_len = 0;

        for (i = 0; i < *len; i++)
        {
            VV_HEX_FROM_BYTE (hex_ciphertext+tot_len, (unsigned int)ciphertext[i]);
            tot_len += 2;
        }

        hex_ciphertext[*len = tot_len] = 0;
        vely_free (ciphertext); // free binary encrypted value
        return hex_ciphertext; // return hex value
    }
    else
    {
        //
        // The encrypted value is binary, not a hex-string, typically for files
        //
        return (char*)ciphertext;
    }

    return ""; // no purpose, eliminate compiler warning about non-return from function
}


// 
// Decrypt string 'ciphertext' and return decrypted value, with output parameter 
// 'len' being the length of this decrypted value returned by the function (excluding zero byte at the end -
// the decrypted value doesn't have to be a string, but it will have a zero byte at the end).
// 'len' is also input parameter - it's the length of input data to decrypt.
// We allocate new memory for decrypted value - caller must de-allocate (this is vely_
// allocation, so you may just leave it to be collected by VELY memory garbage collector.
// 'e' is the encryption context, produced as d_ctx in vely_get_enc_key().
// iv is the nonce (IV), the length of which should be sufficient for the algorithm
// 'is_binary' is 1 if encrypted value was encrypted in binary mode, and 0 if as a hex string.
// Note that ciphertext must be the result of vely_encrypt() 
//
char *vely_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, num *len, num is_binary, unsigned char *iv)
{
   assert (*len>=0);
   unsigned char *cipher_bin = ciphertext;
   if (is_binary == 0)
   {
       cipher_bin = vely_malloc (*len/2 + 2); // actually needs only *len/2, 1 for odd, 1 for null just in case
       //
       // convert ciphertext from hex back to binary
       // (if the encrypted value is hex-string)
       //
       num i;
       num curr_byte = 0;
       for (i = 0; i < *len; i+=2)
       {
           cipher_bin[curr_byte] = ((unsigned int)(VV_CHAR_FROM_HEX(ciphertext[i]))<<4) + (unsigned int)VV_CHAR_FROM_HEX(ciphertext[i+1]);
           curr_byte++;
       }
       cipher_bin[*len = curr_byte] = 0;
   }


   /* plaintext is equal or lesser length than that of cipher_bin*/
   int p_len = *len, f_len = 0;
   unsigned char *plaintext = vely_malloc(p_len);
           
   // reuse decryption context in case multiple decryptions done with the same context
#if OPENSSL_VERSION_MAJOR  >= 3
   EVP_DecryptInit_ex2(e, NULL, NULL, iv, NULL);
#else
   EVP_DecryptInit_ex(e, NULL, NULL, NULL, iv);
#endif
   // decrypt data
   EVP_DecryptUpdate(e, plaintext, &p_len, cipher_bin, *len);
   EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);
                  
   *len = p_len + f_len;
   plaintext[*len] = 0; 

   if (is_binary == 0) vely_free (cipher_bin); // deallocate binary encrypted if allocated
   return (char*)plaintext;
}





// 
// Encode data in Base64. 'in' is the input, of length in_len. 'out' is allocated
// here and is the result of encoding, and the length of it is in out_len (if not NULL).
// Note: all b64 data must be produced on a single line, per openssl docs.
// If in_len is -1, then it become the length of string in.
//
void vely_b64_encode(const char* in, num in_len, char** out, num* olen)
{ 
    VV_TRACE("");
    if (in_len == -1) in_len = strlen (in);
    *out = vely_malloc(4*((in_len+2)/3)+2); 
    num out_len = EVP_EncodeBlock((unsigned char*)(*out), (unsigned char*)in, in_len);
    if (olen != NULL) *olen = out_len;
}
 
// 
// Decode Base64 data. 'in' is b64 data of length 'in_len', and the output is allocated
// as 'out' of length out_len (if not NULL).
// Note: all b64 data must be on a single line, per openssl docs.
// If in_len is -1, then it become the length of string in.
//
void vely_b64_decode (const char* in, num ilen, char** out, num* olen) 
{
    VV_TRACE("");
    if (ilen == -1) ilen = strlen (in);
    *out = vely_malloc(3*ilen/4+4 + 1);
    // remove trailing =s because they are added if the original input is not of size multiple of 3
    num final = 0;
    while (in[ilen-1-final] == '=') final++;
    num out_len = EVP_DecodeBlock((unsigned char*)(*out), (unsigned char*)in, ilen);
    (*out)[out_len - final] = 0; // we added +1 in vely_malloc above to account for 0 always at the end (not part of output length)
    if (olen != NULL) *olen = out_len-final;
}

