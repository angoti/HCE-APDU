package com.professorangoti.hce

//import com.professorangoti.hce.R
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
fun TelaCentralizada() {
    // 1. Definindo a sua paleta de cores
    val corFundo = Color.White
    val verdeClaro = Color(0xFFA5D6A7)
    val verdeEscuro = Color(0xFF3FA14C)// Tom de verde claro agradável
    val vermelho = Color(0xFFE53935)   // Tom de vermelho padrão

    // Surface atua como o "fundo" da nossa tela
    Surface(
        modifier = Modifier.fillMaxSize(),
        color = corFundo
    ) {
        // 2. Column empilha os itens verticalmente
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp),

            verticalArrangement = Arrangement.Center,

            horizontalAlignment = Alignment.CenterHorizontally
        ) {


            Image(
                painter = painterResource(R.drawable.iftm),
                contentDescription = "Logo IF",
                modifier = Modifier.size(150.dp)
            )

            Spacer(modifier = Modifier.height(32.dp))

            Text(
                text = "IF",
                color = vermelho,
                fontSize = 28.sp,
                fontWeight = FontWeight.Bold,
                textAlign = TextAlign.Center
            )

            Text(
                text = "KEY",
                color = verdeEscuro,
                fontSize = 28.sp,
                fontWeight = FontWeight.Bold,
                textAlign = TextAlign.Center
            )

            Spacer(modifier = Modifier.height(12.dp))

            Text(
                text = "Neste aplicativo você acessa sua chave de acesso as trancas eletrônicas do IFTM.",
                color = verdeClaro, // Usando o verde claro no texto também
                fontSize = 18.sp,
                textAlign = TextAlign.Center
            )

            Spacer(modifier = Modifier.height(8.dp))

            Text(
                text = "Aproxime seu telefone á tranca para destrancar a porta.",
                color = Color.Gray, // Usando cinza para dar contraste com o fundo branco
                fontSize = 16.sp,
                textAlign = TextAlign.Center
            )
        }
    }
}

@Preview(showBackground = true)
@Composable
fun TelaCentralizadaPreview() {
    TelaCentralizada()
}