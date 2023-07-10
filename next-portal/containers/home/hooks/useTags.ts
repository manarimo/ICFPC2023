import useSWR from "swr";

const useTags = () => {
  const url = `https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/tags`;
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const data = await response.json();
    return data as {
      tags: string[]
    };
  };
  return useSWR(url, fetcher);
};
export default useTags;
