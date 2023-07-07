import useSWR from "swr";

const useProblemList = () => {
  const url = "/public/problem-index.json";
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const body = await response.json();
    return body as {
      id: string;
      roomWidth: number;
      roomHeight: number;
      numMusicians: number;
      numAttendees: number;
    }[];
  };
  return useSWR(url, fetcher);
};
export default useProblemList;
