const header = document.getElementById("subHeader");

let lastScroll = 0;

window.addEventListener("scroll", () => {
    const currentScroll = window.scrollY;
    if(currentScroll <= 0){
        header.classList.remove("hidden");
    } else if(currentScroll > lastScroll){
        // scroll down
        header.classList.add("hidden");
    } else {
        // scroll up
        header.classList.remove("hidden");
    }
    lastScroll = currentScroll;
});

const container = document.querySelector(".heart-container");

function createHeart() {
    const heart = document.createElement("div");
    heart.classList.add("heart2");

    // posição horizontal aleatória
    heart.style.left = Math.random() * 100 + "vw";

    // tamanho aleatório
    const size = Math.random() * 20 + 10; // 10px a 30px
    heart.style.width = size + "px";
    heart.style.height = size + "px";

    // duração da animação aleatória
    heart.style.animationDuration = (3 + Math.random() * 2) + "s";

    container.appendChild(heart);

    // remover após terminar a animação
    heart.addEventListener("animationend", () => {
        heart.remove();
    });
}

// criar corações a cada 300ms
setInterval(createHeart, 300);