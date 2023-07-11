import useSWR from "swr";

const useTags = () => {
  const url = `http://icfpc2023-manarimo-3mrzsd.s3-website-ap-northeast-1.amazonaws.com/snapshot/tags.json`;
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
