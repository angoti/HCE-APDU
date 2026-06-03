package com.professorangoti.hce

import android.nfc.cardemulation.HostApduService
import android.os.Bundle
import android.util.Log
import javax.crypto.Mac
import javax.crypto.spec.SecretKeySpec

class HceService : HostApduService() {

    companion object {
        private const val TAG = "HceService"

        // Mesma chave do Arduino
        val SECRET_KEY = byteArrayOf(
            0x49, 0x46, 0x54, 0x4D, 0x4E, 0x46, 0x43, 0x4C,
            0x4F, 0x43, 0x4B, 0x32, 0x30, 0x32, 0x35, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
        )

        val SELECT_OK  = byteArrayOf(0x90.toByte(), 0x00)
        val UNKNOWN    = byteArrayOf(0x6A.toByte(), 0x82.toByte())
        val ERROR      = byteArrayOf(0x69.toByte(), 0x00.toByte())

        const val INS_SELECT    = 0xA4.toByte()
        const val INS_CHALLENGE = 0x20.toByte()
    }

    override fun processCommandApdu(apdu: ByteArray, extras: Bundle?): ByteArray {
        val hex = apdu.joinToString(" ") { "%02X".format(it) }
        Log.d(TAG, "APDU recebido: $hex")

        if (apdu.size < 4) return UNKNOWN

        return when (apdu[1]) {
            INS_SELECT -> {
                Log.d(TAG, "SELECT AID - OK")
                SELECT_OK
            }
            INS_CHALLENGE -> {
                if (apdu.size < 13) return ERROR
                val nonce = apdu.copyOfRange(5, 13)
                Log.d(TAG, "CHALLENGE recebido, nonce: ${nonce.joinToString(" ") { "%02X".format(it) }}")

                try {
                    val mac = Mac.getInstance("HmacSHA256")
                    mac.init(SecretKeySpec(SECRET_KEY, "HmacSHA256"))
                    val hmac = mac.doFinal(nonce)

                    Log.d(TAG, "HMAC calculado: ${hmac.joinToString(" ") { "%02X".format(it) }}")

                    // Retorna HMAC (32 bytes) + SW 90 00
                    hmac + SELECT_OK
                } catch (e: Exception) {
                    Log.e(TAG, "Erro no HMAC: ${e.message}")
                    ERROR
                }
            }
            else -> {
                Log.d(TAG, "Comando desconhecido")
                UNKNOWN
            }
        }
    }

    override fun onDeactivated(reason: Int) {
        Log.d(TAG, "Desativado: $reason")
    }
}