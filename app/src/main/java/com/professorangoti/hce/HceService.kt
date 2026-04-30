package com.professorangoti.hce

import android.nfc.cardemulation.HostApduService
import android.os.Bundle
import android.util.Log

class HceService : HostApduService() {

    companion object {
        private const val TAG = "HceService"
        val SELECT_OK = byteArrayOf(0x90.toByte(), 0x00)
        val UNKNOWN   = byteArrayOf(0x6A.toByte(), 0x82.toByte())
    }

    override fun processCommandApdu(apdu: ByteArray, extras: Bundle?): ByteArray {
        val hex = apdu.joinToString(" ") { "%02X".format(it) }
        Log.d(TAG, "APDU recebido: $hex")

        if (apdu.size >= 5 && apdu[1] == 0xA4.toByte()) {
            Log.d(TAG, "SELECT AID - respondendo 90 00")
            return SELECT_OK
        }

        Log.d(TAG, "Comando desconhecido - respondendo 6A 82")
        return UNKNOWN
    }

    override fun onDeactivated(reason: Int) {
        Log.d(TAG, "Desativado: $reason")
    }
}