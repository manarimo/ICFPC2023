import type { AppProps } from "next/app";

export default function App({ Component, pageProps }: AppProps) {
  return (
    <>
      <Component {...pageProps} />
      <style jsx global>{`
        body {
          margin: 0;
          color: #333;
          font-family: YakuHanJP, "Hiragino Sans", "Hiragino Kaku Gothic ProN",
            "Noto Sans JP", Meiryo, sans-serif, "IPAゴシック";
        }
      `}</style>
    </>
  );
}
